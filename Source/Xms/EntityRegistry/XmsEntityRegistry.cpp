// Copyright (c) 2025 Xist.GG

#include "XmsEntityRegistry.h"

#include "XmsLog.h"
#include "Engine/World.h"

UXmsRegistrySubsystem* UXmsRegistrySubsystem::Get(TNotNull<const UWorld*> World)
{
	UXmsRegistrySubsystem* Result = World->GetSubsystem<UXmsRegistrySubsystem>();
	return Result;
}

UXmsRegistrySubsystem& UXmsRegistrySubsystem::GetChecked(TNotNull<const UWorld*> World)
{
	UXmsRegistrySubsystem* Result = Get(World);
	check(Result);
	return *Result;
}

// Set Class Defaults
UXmsRegistrySubsystem::UXmsRegistrySubsystem()
{
}

void UXmsRegistrySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UXmsRegistrySubsystem::Deinitialize()
{
	Super::Deinitialize();
}

void UXmsRegistrySubsystem::Tick(float DeltaTime)
{
	// DO NOT CALL UNIMPLEMENTED SUPER UMassTickableSubsystemBase::Tick
	//-Super::Tick(DeltaTime);

	HandleMassQueues();
}

TStatId UXmsRegistrySubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UXmsRegistrySubsystem, STATGROUP_Tickables);
}

FDelegateHandle UXmsRegistrySubsystem::SubscribeToEntitiesCreated(FEntityContextEvent::FDelegate&& Delegate)
{
	checkf(not bIsInCreateBroadcast, TEXT("Dev Error: DO NOT modify Create delegate during Create broadcast"));

	FScopeLock ScopeLock(&CriticalCreateEvent);
	return OnEntitiesCreated.Add(Delegate);
}

bool UXmsRegistrySubsystem::UnsubscribeFromEntitiesCreated(FDelegateHandle DelegateHandle)
{
	checkf(not bIsInCreateBroadcast, TEXT("Dev Error: DO NOT modify Create delegate during Create broadcast"));

	FScopeLock ScopeLock(&CriticalCreateEvent);
	return OnEntitiesCreated.Remove(DelegateHandle);
}

FDelegateHandle UXmsRegistrySubsystem::SubscribeToEntitiesDestroyed(FEntityContextEvent::FDelegate&& Delegate)
{
	checkf(not bIsInDestroyBroadcast, TEXT("Dev Error: DO NOT modify Destroy delegate during Destroy broadcast"));

	FScopeLock ScopeLock(&CriticalDestroyEvent);
	return OnEntitiesDestroyed.Add(Delegate);
}

bool UXmsRegistrySubsystem::UnsubscribeFromEntitiesDestroyed(FDelegateHandle DelegateHandle)
{
	checkf(not bIsInDestroyBroadcast, TEXT("Dev Error: DO NOT modify Destroy delegate during Destroy broadcast"));

	FScopeLock ScopeLock(&CriticalDestroyEvent);
	return OnEntitiesDestroyed.Remove(DelegateHandle);
}

int32 UXmsRegistrySubsystem::GetEntitiesByMetaType(const EXmsEntityMetaType& MetaType, TArray<FMassEntityHandle>& OutEntities) const
{
	QUICK_SCOPE_CYCLE_COUNTER(UXmsRegistrySubsystem_GetEntitiesByMetaType);

	UE::TReadScopeLock ReadLock (EntityLockRW);

	const int32 OrigNum = OutEntities.Num();
	for (const auto& Tuple : MetaEntities)
	{
		if (Tuple.Value.MetaType == MetaType)
		{
			OutEntities.Add(Tuple.Key);
		}
	}
	return OutEntities.Num() - OrigNum;
}

void UXmsRegistrySubsystem::MassOnEntitiesCreated(const TArray<FEntityContext>& Entities)
{
	QUICK_SCOPE_CYCLE_COUNTER(UXmsRegistrySubsystem);

	if (IsInGameThread())
	{
		NativeOnEntitiesCreated(Entities);
		return;
	}

	CreateQueue.Enqueue(Entities);
}

void UXmsRegistrySubsystem::MassOnEntitiesDestroyed(const TArray<FEntityContext>& Entities)
{
	QUICK_SCOPE_CYCLE_COUNTER(UXmsRegistrySubsystem);

	if (IsInGameThread())
	{
		NativeOnEntitiesDestroyed(Entities);
		return;
	}

	DestroyQueue.Enqueue(Entities);
}

void UXmsRegistrySubsystem::HandleMassQueues()
{
	QUICK_SCOPE_CYCLE_COUNTER(UXmsRegistrySubsystem_Dequeue);

	checkSlow(IsInGameThread());

	TArray<FEntityContext> Entities;

	// Free resources from now-deleted Entities
	while (DestroyQueue.Dequeue(OUT Entities))
	{
		NativeOnEntitiesDestroyed(Entities);
	}

	// Allocate resources for new Entities
	while (CreateQueue.Dequeue(OUT Entities))
	{
		NativeOnEntitiesCreated(Entities);
	}
}

void UXmsRegistrySubsystem::NativeOnEntitiesCreated(const TArray<FEntityContext>& Entities)
{
	QUICK_SCOPE_CYCLE_COUNTER(UXmsRegistrySubsystem_EntitiesCreated);

	checkSlow(IsInGameThread());

	for (int32 i = 0; i < Entities.Num(); ++i)
	{
		const FEntityContext& EntityContext = Entities[i];
		GameOnEntityCreated(EntityContext);
	}

	{
		FScopeLock ScopeLock(&CriticalCreateEvent);
		bIsInCreateBroadcast = true;
		OnEntitiesCreated.Broadcast(Entities);
		bIsInCreateBroadcast = false;
	}
}

void UXmsRegistrySubsystem::NativeOnEntitiesDestroyed(const TArray<FEntityContext>& Entities)
{
	QUICK_SCOPE_CYCLE_COUNTER(UXmsRegistrySubsystem_EntitiesDestroyed);

	checkSlow(IsInGameThread());

	for (int32 i = 0; i < Entities.Num(); ++i)
	{
		const FEntityContext& EntityContext = Entities[i];
		GameOnEntityDestroyed(EntityContext);
	}

	{
		FScopeLock ScopeLock(&CriticalDestroyEvent);
		bIsInDestroyBroadcast = true;
		OnEntitiesDestroyed.Broadcast(Entities);
		bIsInDestroyBroadcast = false;
	}
}

void UXmsRegistrySubsystem::GameOnEntityCreated(const FEntityContext& EntityContext)
{
	QUICK_SCOPE_CYCLE_COUNTER(UXmsRegistrySubsystem_OnCreated);

	checkSlow(IsInGameThread());

	const FMassEntityHandle& EntityHandle = EntityContext.Entity;
	const FCSFXms_MetaData& MetaData = EntityContext.MetaData;

	checkf(EntityHandle.IsSet(), TEXT("EntityHandle must be set"));
	checkf(MetaData.IsValid(), TEXT("Entity was created with invalid MetaData"));

	// Add Entity MetaData
	{
		UE::TWriteScopeLock WriteLock (EntityLockRW);
		MetaEntities.Add(EntityHandle, MetaData);
	}

#if WITH_XMS_DEBUG
	UE_VLOG_UELOG(this, LogXmsRegistry, Verbose, TEXT("%hs: Add Entity [%s] to Registry (MetaType: %s)"),
		__FUNCTION__, *EntityHandle.DebugGetDescription(), *UEnum::GetDisplayValueAsText(MetaData.MetaType).ToString());
#endif
}

void UXmsRegistrySubsystem::GameOnEntityDestroyed(const FEntityContext& EntityContext)
{
	QUICK_SCOPE_CYCLE_COUNTER(UXmsRegistrySubsystem_OnDestroyed);

	checkSlow(IsInGameThread());

	const FMassEntityHandle& EntityHandle = EntityContext.Entity;

	FCSFXms_MetaData MetaDataCopy;
	bool bWasRemoved;
	{
		UE::TWriteScopeLock WriteLock (EntityLockRW);
		bWasRemoved = MetaEntities.RemoveAndCopyValue(EntityHandle, OUT MetaDataCopy);
	}

#if WITH_XMS_DEBUG
	if (bWasRemoved)
	{
		UE_VLOG_UELOG(this, LogXmsRegistry, Verbose, TEXT("%hs: Remove Entity [%s] from Registry (MetaType: %s)"),
			__FUNCTION__, *EntityHandle.DebugGetDescription(), *UEnum::GetDisplayValueAsText(MetaDataCopy.MetaType).ToString());
	}
#endif
}
