// Copyright (c) 2025 Xist.GG

#pragma once

#include "MassEntityHandle.h"
#include "MassSubsystemBase.h"
#include "XmsEntityMetaData.h"
#include "Containers/Queue.h"
#include "Misc/TransactionallySafeRWLock.h"
#include "XmsEntityRegistry.generated.h"

/**
 * FXmsT_Registry
 *
 * An Entity must be tagged with this tag to be seen by the XmsEntityRegistry observer
 * processors, thus any Entity that wants to be in the Registry requires this tag.
 */
USTRUCT()
struct FXmsT_Registry
	: public FMassTag
{
	GENERATED_BODY()
};

/**
 * UXmsRegistrySubsystem
 */
UCLASS(Config=Xms)
class XMS_API UXmsRegistrySubsystem
	: public UMassTickableSubsystemBase
{
	GENERATED_BODY()

public:
	static UXmsRegistrySubsystem* Get(TNotNull<const UWorld*> World);
	static UXmsRegistrySubsystem& GetChecked(TNotNull<const UWorld*> World);

	struct FEntityContext
	{
		FMassEntityHandle Entity;
		FXmsCSF_MetaData MetaData;
	};

	DECLARE_MULTICAST_DELEGATE_OneParam(FOnEntityContextEvent, const TArray<FEntityContext>& /*EntityContexts*/);

	FDelegateHandle SubscribeToEntitiesCreated(FOnEntityContextEvent::FDelegate&& Delegate);
	bool UnsubscribeFromEntitiesCreated(FDelegateHandle DelegateHandle);

	FDelegateHandle SubscribeToEntitiesDestroyed(FOnEntityContextEvent::FDelegate&& Delegate);
	bool UnsubscribeFromEntitiesDestroyed(FDelegateHandle DelegateHandle);

	// Set Class Defaults
	UXmsRegistrySubsystem();

	//~Begin UWorldSubsystem interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	//~End UWorldSubsystem interface

	//~Begin UTickableWorldSubsystem interface
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;
	//~End UTickableWorldSubsystem interface

	/**
	 * Get an array of all Entities matching the MetaType
	 * @param MetaType 
	 * @param OutEntities OUTPUT array into which to append Entities
	 * @return Number of Entities added to OutEntities
	 */
	int32 GetEntitiesByMetaType(const EXmsEntityMetaType& MetaType, TArray<FMassEntityHandle>& OutEntities) const;

	/**
	 * Called by a Mass Observer Processor when Registry Entities are Created
	 * @param Entities One or more newly created Entities
	 */
	void MassOnEntitiesCreated(const TArray<FEntityContext>& Entities);

	/**
	 * Called by a Mass Observer Processor when Registry Entities are Destroyed
	 * @param Entities One or more newly created Entities
	 */
	void MassOnEntitiesDestroyed(const TArray<FEntityContext>& Entities);

protected:
	/**
	 * Handle delayed processing of off-GameThread Mass events on the Game thread
	 */
	void HandleMassQueues();

	/**
	 * GameThread handler called whenever new Entities have been created.
	 *
	 * This in turn calls GameOnEntityCreated() for each Entity.
	 * 
	 * @param Entities Array of new Entities
	 */
	void NativeOnEntitiesCreated(const TArray<FEntityContext>& Entities);

	/**
	 * GameThread handler called whenever Entities have been destroyed
	 *
	 * This in turn calls GameOnEntityDestroyed() for each Entity.
	 * 
	 * @param Entities Array of now-destroyed Entities
	 */
	void NativeOnEntitiesDestroyed(const TArray<FEntityContext>& Entities);

	/**
	 * Called on the Game Thread whenever a new Mass Entity has been created.
	 * Will be delayed to the next PrePhysics tick if called off the Game thread.
	 * @param EntityContext Newly created Entity
	 */
	void GameOnEntityCreated(const FEntityContext& EntityContext);

	/**
	 * Called on the Game Thread whenever a Mass Entity has been destroyed.
	 * Will be delayed to the next PrePhysics tick if called off the Game thread.
	 * @param EntityContext Now destroyed Entity
	 */
	void GameOnEntityDestroyed(const FEntityContext& EntityContext);

private:
	UPROPERTY(Transient)
	TMap<FMassEntityHandle, FXmsCSF_MetaData> EntityMeta;

	mutable FTransactionallySafeRWLock EntityLockRW;

	TQueue<TArray<FEntityContext>> CreateQueue;
	TQueue<TArray<FEntityContext>> DestroyQueue;

	FOnEntityContextEvent OnEntitiesCreated;
	FOnEntityContextEvent OnEntitiesDestroyed;
};

template<>
struct TMassExternalSubsystemTraits<UXmsRegistrySubsystem> final
{
	enum
	{
		GameThreadOnly = false,  // YES read thread-safe
		ThreadSafeWrite = true,  // YES write thread-safe
	};
};
