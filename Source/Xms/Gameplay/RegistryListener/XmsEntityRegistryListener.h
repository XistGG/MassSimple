// Copyright (c) 2025 Xist.GG LLC

#pragma once

#include "EntityRegistry/XmsEntityRegistry.h"
#include "GameFramework/Actor.h"
#include "XmsEntityRegistryListener.generated.h"

/**
 * AXmsEntityRegistryListener
 */
UCLASS(Abstract, Config=Xms)
class XMS_API AXmsEntityRegistryListener
	: public AActor
{
	GENERATED_BODY()

public:
	// Set Class Defaults
	AXmsEntityRegistryListener(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~Begin UObject interface
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~End UObject interface

protected:
	/** Set this to the list of meta types for which you want events to fire */
	UPROPERTY(EditAnywhere, Category=Xms)
	TArray<EXmsEntityMetaType> ObservedMetaTypes;

	/**
	 * 
	 * @param MetaType 
	 * @param EntityContexts 
	 */
	virtual void NativeOnObservedEntitiesCreated(const EXmsEntityMetaType& MetaType, const TArray<UXmsRegistrySubsystem::FEntityContext>& EntityContexts);

	/**
	 * 
	 * @param MetaType 
	 * @param EntityContexts 
	 */
	virtual void NativeOnObservedEntitiesDestroyed(const EXmsEntityMetaType& MetaType, const TArray<UXmsRegistrySubsystem::FEntityContext>& EntityContexts);

	/**
	 * Callback executed by UXmsRegistrySubsystem when new entities are created
	 * @param EntityContexts Array of new Entities
	 */
	void NativeOnEntitiesCreated(const TArray<UXmsRegistrySubsystem::FEntityContext>& EntityContexts);

	/**
	 * Callback executed by UXmsRegistrySubsystem when Entities are destroyed
	 * @param EntityContexts Array of Entities that were destroyed
	 */
	void NativeOnEntitiesDestroyed(const TArray<UXmsRegistrySubsystem::FEntityContext>& EntityContexts);

private:
	FDelegateHandle OnEntitiesCreatedHandle;
	FDelegateHandle OnEntitiesDestroyedHandle;
};
