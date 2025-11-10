// Copyright (c) 2025 Xist.GG LLC

#pragma once

#include "MassEntityHandle.h"
#include "EntityRegistry/XmsEntityMetaData.h"
#include "GameFramework/Actor.h"
#include "XmsEntityAutoBuilder.generated.h"

namespace UE::Mass
{
	struct FEntityBuilder;
}

/**
 * AXmsEntityAutoBuilder
 */
UCLASS()
class XMS_API AXmsEntityAutoBuilder
	: public AActor
{
	GENERATED_BODY()

public:
	// Set Class Defaults
	AXmsEntityAutoBuilder(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~Begin UObject interface
	virtual void Tick(float DeltaSeconds) override;
	//~End UObject interface

	/**
	 * @return True if AutoBuild is enabled, else False.
	 */
	bool IsAutoBuildEnabled() const { return AutoBuildIntervalSeconds >= 0.; }

protected:
	/**
	 * MetaType of the Entity to build
	 */
	UPROPERTY(EditAnywhere, Category=Xms)
	EXmsEntityMetaType EntityMetaType;

	/**
	 * Interval (in seconds) between Entity auto-builds.
	 * 
	 *   X>0  = at most one Entity will be built every X game seconds
	 *   Zero = one Entity will be built every tick.
	 *   X<0  = AutoBuild is disabled.
	 */
	UPROPERTY(EditAnywhere, Category=Xms)
	float AutoBuildIntervalSeconds;

	virtual FMassEntityHandle BuildEntity();

	virtual void SetupEntityBuilder(UE::Mass::FEntityBuilder& Builder);

private:
	float TimeToNextAutoBuild = -1.;
};
