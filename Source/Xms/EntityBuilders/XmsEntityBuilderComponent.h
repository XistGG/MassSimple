// Copyright (c) 2025 Xist.GG LLC

#pragma once

#include "MassEntityHandle.h"
#include "Attributes/Lifespan/XmsLifespan.h"
#include "EntityRegistry/XmsEntityMetaData.h"
#include "Components/ActorComponent.h"
#include "XmsEntityBuilderComponent.generated.h"

namespace UE::Mass
{
	struct FEntityBuilder;
}

struct FXmsF_Transform;

/**
 * UXmsEntityBuilderComponent
 *
 * One of these components exists on the Character Blueprint in Editor.
 * This builds the Wisp Entities around the Player Pawn.
 */
UCLASS(meta=(BlueprintSpawnableComponent))
class XMS_API UXmsEntityBuilderComponent
	: public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UXmsEntityBuilderComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~Begin UActorComponent interface
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	//~End UActorComponent interface

	/**
	 * @return True if AutoBuild is enabled, else False.
	 */
	bool IsAutoBuildEnabled() const { return bAutoBuildEnabled && AutoBuildIntervalSeconds >= 0.; }

public:
	/**
	 * MetaType of the Entity to build
	 */
	UPROPERTY(EditAnywhere, Category=Xms)
	EXmsEntityMetaType EntityMetaType;

	/**
	 * Initial value for the Entity's lifespan fragment
	 */
	UPROPERTY(EditAnywhere, Category=Xms)
	FXmsF_Lifespan EntityLifespan;

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

protected:
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

private:
	/** Time (seconds) until the next AutoBuild should trigger (only valid when IsAutoBuildEnabled()==true) */
	float TimeToNextAutoBuild = -1.;
};
