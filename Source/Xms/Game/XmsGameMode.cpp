// Copyright (c) 2025 Xist.GG LLC

#include "XmsGameMode.h"
#include "XmsPlayerController.h"
#include "XmsCharacter.h"

// Set Class Defaults
AXmsGameMode::AXmsGameMode(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	DefaultPawnClass = AXmsCharacter::StaticClass();
	PlayerControllerClass = AXmsPlayerController::StaticClass();
}

void AXmsGameMode::PostInitProperties()
{
	Super::PostInitProperties();

	if (OverridePawnClass)
	{
		// Override default class based on INI
		DefaultPawnClass = OverridePawnClass;
	}

	if (OverridePlayerControllerClass)
	{
		// Override default class based on INI
		PlayerControllerClass = OverridePlayerControllerClass;
	}
}
