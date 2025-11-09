// Copyright (c) 2025 Xist.GG LLC

#pragma once

#include "GameFramework/GameModeBase.h"
#include "XmsGameMode.generated.h"

/**
 * AXmsGameMode
 *
 * Blueprint classes are assigned as game defaults via Config INI.
 *
 * @see Config/DefaultXms.ini
 */
UCLASS(Blueprintable, Config=Xms)
class AXmsGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	// Set Class Defaults
	AXmsGameMode(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~Begin UObject interface
	virtual void PostInitProperties() override;
	//~End UObject interface

protected:
	UPROPERTY(Config, EditDefaultsOnly)
	TSubclassOf<APawn> OverridePawnClass;

	UPROPERTY(Config, EditDefaultsOnly)
	TSubclassOf<APlayerController> OverridePlayerControllerClass;
};
