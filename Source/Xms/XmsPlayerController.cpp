// Copyright (c) 2025 Xist.GG LLC

#include "XmsPlayerController.h"
#include "GameFramework/Pawn.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "Engine/World.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "XmsLog.h"
#include "Engine/LocalPlayer.h"

// Set class defaults
AXmsPlayerController::AXmsPlayerController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Default;
	CachedDestination = FVector::ZeroVector;
	ShortPressThreshold = 0.2f;  // 200 ms
}

void AXmsPlayerController::PostInitProperties()
{
	Super::PostInitProperties();

	ApplyIniSettings();
}

void AXmsPlayerController::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	// Help the dev understand if they've misconfigured a required setting
	ensureMsgf(FXCursor != nullptr, TEXT("%hs: You didn't configure a valid FXCursor"), __FUNCTION__);
	ensureMsgf(DefaultMappingContext != nullptr, TEXT("%hs: You didn't configure a valid DefaultMappingContext"), __FUNCTION__);
	ensureMsgf(SetDestinationClickAction != nullptr, TEXT("%hs: You didn't configure a valid SetDestinationClickAction"), __FUNCTION__);
	ensureMsgf(SetDestinationTouchAction != nullptr, TEXT("%hs: You didn't configure a valid SetDestinationTouchAction"), __FUNCTION__);
}

void AXmsPlayerController::SetupInputComponent()
{
	// set up gameplay key bindings
	Super::SetupInputComponent();

	// Add Input Mapping Context
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		Subsystem->AddMappingContext(DefaultMappingContext, 0);
	}

	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
	{
		// Setup mouse input events
		EnhancedInputComponent->BindAction(SetDestinationClickAction, ETriggerEvent::Started, this, &AXmsPlayerController::OnInputStarted);
		EnhancedInputComponent->BindAction(SetDestinationClickAction, ETriggerEvent::Triggered, this, &AXmsPlayerController::OnSetDestinationTriggered);
		EnhancedInputComponent->BindAction(SetDestinationClickAction, ETriggerEvent::Completed, this, &AXmsPlayerController::OnSetDestinationReleased);
		EnhancedInputComponent->BindAction(SetDestinationClickAction, ETriggerEvent::Canceled, this, &AXmsPlayerController::OnSetDestinationReleased);

		// Setup touch input events
		EnhancedInputComponent->BindAction(SetDestinationTouchAction, ETriggerEvent::Started, this, &AXmsPlayerController::OnInputStarted);
		EnhancedInputComponent->BindAction(SetDestinationTouchAction, ETriggerEvent::Triggered, this, &AXmsPlayerController::OnTouchTriggered);
		EnhancedInputComponent->BindAction(SetDestinationTouchAction, ETriggerEvent::Completed, this, &AXmsPlayerController::OnTouchReleased);
		EnhancedInputComponent->BindAction(SetDestinationTouchAction, ETriggerEvent::Canceled, this, &AXmsPlayerController::OnTouchReleased);
	}
	else
	{
		UE_LOG(LogXms, Error, TEXT("'%s' Failed to find an Enhanced Input Component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void AXmsPlayerController::OnInputStarted()
{
	StopMovement();
}

// Triggered every frame when the input is held down
void AXmsPlayerController::OnSetDestinationTriggered()
{
	// We flag that the input is being pressed
	FollowTime += GetWorld()->GetDeltaSeconds();
	
	// We look for the location in the world where the player has pressed the input
	FHitResult Hit;
	bool bHitSuccessful = false;
	if (bIsTouch)
	{
		bHitSuccessful = GetHitResultUnderFinger(ETouchIndex::Touch1, ECollisionChannel::ECC_Visibility, true, Hit);
	}
	else
	{
		bHitSuccessful = GetHitResultUnderCursor(ECollisionChannel::ECC_Visibility, true, Hit);
	}

	// If we hit a surface, cache the location
	if (bHitSuccessful)
	{
		CachedDestination = Hit.Location;
	}
	
	// Move towards mouse pointer or touch
	APawn* ControlledPawn = GetPawn();
	if (ControlledPawn != nullptr)
	{
		FVector WorldDirection = (CachedDestination - ControlledPawn->GetActorLocation()).GetSafeNormal();
		ControlledPawn->AddMovementInput(WorldDirection, 1.0, false);
	}
}

void AXmsPlayerController::OnSetDestinationReleased()
{
	// If it was a short press
	if (FollowTime <= ShortPressThreshold)
	{
		// We move there and spawn some particles
		UAIBlueprintHelperLibrary::SimpleMoveToLocation(this, CachedDestination);
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, FXCursor, CachedDestination, FRotator::ZeroRotator, FVector(1.f, 1.f, 1.f), true, true, ENCPoolMethod::None, true);
	}

	FollowTime = 0.f;
}

// Triggered every frame when the input is held down
void AXmsPlayerController::OnTouchTriggered()
{
	bIsTouch = true;
	OnSetDestinationTriggered();
}

void AXmsPlayerController::OnTouchReleased()
{
	bIsTouch = false;
	OnSetDestinationReleased();
}

void AXmsPlayerController::ApplyIniSettings()
{
	if (FXCursor == nullptr
		&& not FXCursorPath.IsEmpty())
	{
		FXCursor = Cast<UNiagaraSystem>(StaticLoadObject(UNiagaraSystem::StaticClass(), nullptr,
			FXCursorPath, nullptr, LOAD_None, nullptr));
		UE_CLOG(FXCursor == nullptr, LogXms, Error, TEXT("%hs: FXCursorPath [%s] is not valid"), __FUNCTION__, *FXCursorPath);
	}

	if (DefaultMappingContext == nullptr
		&& not IMCPath.IsEmpty())
	{
		DefaultMappingContext = Cast<UInputMappingContext>(StaticLoadObject(UInputMappingContext::StaticClass(), nullptr,
			IMCPath, nullptr, LOAD_None, nullptr));
		UE_CLOG(DefaultMappingContext == nullptr, LogXms, Error, TEXT("%hs: IMCPath [%s] is not valid"), __FUNCTION__, *IMCPath);
	}

	if (SetDestinationClickAction == nullptr
		&& not SetDestClickActionPath.IsEmpty())
	{
		SetDestinationClickAction = Cast<UInputAction>(StaticLoadObject(UInputAction::StaticClass(), nullptr,
			SetDestClickActionPath, nullptr, LOAD_None, nullptr));
		UE_CLOG(SetDestinationClickAction == nullptr, LogXms, Error, TEXT("%hs: SetDestClickActionPath [%s] is not valid"), __FUNCTION__, *SetDestClickActionPath);
	}

	if (SetDestinationTouchAction == nullptr
		&& not SetDestTouchActionPath.IsEmpty())
	{
		SetDestinationTouchAction = Cast<UInputAction>(StaticLoadObject(UInputAction::StaticClass(), nullptr,
			SetDestTouchActionPath, nullptr, LOAD_None, nullptr));
		UE_CLOG(SetDestinationTouchAction == nullptr, LogXms, Error, TEXT("%hs: SetDestTouchActionPath [%s] is not valid"), __FUNCTION__, *SetDestTouchActionPath);
	}
}
