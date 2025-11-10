// Copyright (c) 2025 Xist.GG LLC

#include "XmsEntityAutoBuilder.h"

#include "MassEntityBuilder.h"
#include "MassEntityManager.h"
#include "MassEntityUtils.h"
#include "XmsLog.h"
#include "Engine/World.h"
#include "EntityRegistry/XmsEntityMetaData.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Misc/XmsFragments.h"

// Sets default values
AXmsEntityAutoBuilder::AXmsEntityAutoBuilder(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	SetHidden(true);

	EntityMetaType = EXmsEntityMetaType::Wisp;
	AutoBuildIntervalSeconds = 1.;
}

void AXmsEntityAutoBuilder::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// If it's time to auto-build a new Entity, then do it now.

	TimeToNextAutoBuild -= DeltaSeconds;

	if (IsAutoBuildEnabled()
		&& TimeToNextAutoBuild <= 0.)
	{
		BuildEntity();
		TimeToNextAutoBuild = AutoBuildIntervalSeconds;
	}
}

FMassEntityHandle AXmsEntityAutoBuilder::BuildEntity()
{
	UE_VLOG_UELOG(this, LogXmsBuilder, Verbose, TEXT("%hs: %s: Building Entity"),
		__FUNCTION__, *GetClass()->GetName());

	const UWorld* World = GetWorld();
	check(World);

	FMassEntityManager& EntityManager = UE::Mass::Utils::GetEntityManagerChecked(*World);
	UE::Mass::FEntityBuilder Builder = EntityManager.MakeEntityBuilder();

	// Set up Entity debug identification so we know which child class created any given Entity
	FMassArchetypeCreationParams ArchetypeCreationParams;
	ArchetypeCreationParams.DebugName = GetClass()->GetFName();
	Builder.ConfigureArchetypeCreation(ArchetypeCreationParams);

	// Allow child classes to customize Builder setup
	SetupEntityBuilder(OUT Builder);

	const FMassEntityHandle Entity = Builder.Commit();
	return Entity;
}

void AXmsEntityAutoBuilder::SetupEntityBuilder(UE::Mass::FEntityBuilder& Builder)
{
	// Set up Entity MetaData fragment
	FXmsCSF_MetaData MetaData {
		.MetaType = EntityMetaType,
	};
	Builder.Add<FXmsCSF_MetaData>(MetaData);

	// Warn if/when the MetaData is invalid
	UE_CVLOG_UELOG(not MetaData.IsValid(), this, LogXmsBuilder, Warning,
		TEXT("%hs: %s: Invalid MetaData for reserved Entity [%s]"),
		__FUNCTION__, *GetClass()->GetName(), *Builder.GetEntityHandle().DebugGetDescription());

	// Mass Observers apparently cannot observe Const Shared Fragments for created Entities,
	// so we will use this Tag for Observer compatibility.
	Builder.Add<FXmsT_Registry>();

	// Set the transform for the Entity
	FXmsF_Transform EntityTransform;
	InitEntityTransform(OUT EntityTransform);
	Builder.Add<FXmsF_Transform>(EntityTransform);
}

void AXmsEntityAutoBuilder::InitEntityTransform(FXmsF_Transform& OutTransform)
{
	const UWorld* World = GetWorld();
	check(World);

	if (const APlayerController* PC = World->GetFirstPlayerController();
		const APawn* Pawn = PC ? PC->GetPawn() : nullptr)
	{
		OutTransform.Location = Pawn->GetActorLocation();
		OutTransform.Rotation = Pawn->GetActorRotation();
		OutTransform.Scale3D = FVector::OneVector;
		return;
	}

	// Ensure OutTransform is a default transform
	OutTransform = FXmsF_Transform();
}
