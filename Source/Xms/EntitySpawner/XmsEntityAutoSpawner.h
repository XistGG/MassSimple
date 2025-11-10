// Copyright (c) 2025 Xist.GG LLC

#pragma once

#include "MassEntityHandle.h"
#include "EntityRegistry/XmsEntityMetaData.h"
#include "GameFramework/Actor.h"
#include "XmsEntityAutoSpawner.generated.h"

namespace UE::Mass
{
	struct FEntityBuilder;
}

/**
 * AXmsEntitySpawner
 */
UCLASS()
class XMS_API AXmsEntityAutoSpawner
	: public AActor
{
	GENERATED_BODY()

public:
	// Set Class Defaults
	AXmsEntityAutoSpawner(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~Begin UObject interface
	virtual void Tick(float DeltaSeconds) override;
	//~End UObject interface

protected:
	UPROPERTY(EditAnywhere, Category=Xms)
	EXmsEntityMetaType EntityMetaType;

	UPROPERTY(EditAnywhere, Category=Xms)
	float SpawnIntervalSeconds;

	virtual FMassEntityHandle SpawnEntity();

	virtual void SetupEntityBuilder(UE::Mass::FEntityBuilder& Builder);

private:
	float TimeToNextSpawn = -1.;
};
