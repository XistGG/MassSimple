// Copyright (c) 2025 Xist.GG LLC

#pragma once

#include "MassEntityHandle.h"
#include "EntityRegistry/XmsEntityMetaData.h"
#include "GameFramework/Volume.h"
#include "XmsEntityTreeBuilder.generated.h"

namespace UE::Mass
{
	struct FEntityBuilder;
}

struct FXmsF_Transform;

/**
 * AXmsEntityTreeBuilder
 *
 * This Actor is spawned into the default Map. Its bounds are set over the Gameplay area
 * so the instance knows where to build Tree Entities.
 */
UCLASS()
class XMS_API AXmsEntityTreeBuilder
	: public AVolume
{
	GENERATED_BODY()

public:
	// Set Class Defaults
	AXmsEntityTreeBuilder(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~Begin UObject interface
	virtual void Tick(float DeltaSeconds) override;
	//~End UObject interface

	/**
	 * @return True if AutoBuild is enabled, else False.
	 */
	bool IsAutoBuildEnabled() const { return bAutoBuildEnabled && AutoBuildIntervalSeconds >= 0.; }

protected:
	/**
	 * Hardcoded Z coordinate where to build Trees
	 */
	UPROPERTY(EditAnywhere, Category=Xms)
	double BuildWorldZ;

	/**
	 * MetaType of the Entity to build
	 */
	UPROPERTY(EditAnywhere, Category=Xms)
	EXmsEntityMetaType EntityMetaType;

	/**
	 * When True, AutoBuild is enabled. When False, AutoBuild is disabled.
	 */
	UPROPERTY(EditAnywhere, Category=Xms)
	bool bAutoBuildEnabled;

	/**
	 * Interval (in seconds) between Entity auto-builds.
	 * 
	 *   X>0  = at most one Entity will be built every X game seconds
	 *   Zero = one Entity will be built every tick.
	 *   X<0  = AutoBuild is disabled.
	 */
	UPROPERTY(EditAnywhere, Category=Xms)
	float AutoBuildIntervalSeconds;

	/**
	 * Build an Entity
	 * @return Mass Entity Handle for the newly built Entity
	 */
	virtual FMassEntityHandle BuildEntity();

	/**
	 * Configure the Mass Entity Builder for whatever Entity we will build
	 * @param Builder 
	 */
	virtual void SetupEntityBuilder(UE::Mass::FEntityBuilder& IN OUT Builder);

	/**
	 * Get the transform the Entity will be built with
	 * @param OutTransform 
	 */
	virtual void InitEntityTransform(FXmsF_Transform& OUT OutTransform);

private:
	/** Time (seconds) until the next AutoBuild should trigger (only valid when IsAutoBuildEnabled()==true) */
	float TimeToNextAutoBuild = -1.;
};
