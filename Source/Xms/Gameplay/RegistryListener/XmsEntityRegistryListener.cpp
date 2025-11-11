// Copyright (c) 2025 Xist.GG LLC

#include "XmsEntityRegistryListener.h"

#include "XmsLog.h"
#include "EntityRegistry/XmsEntityRegistry.h"

// Set Class Defaults
AXmsEntityRegistryListener::AXmsEntityRegistryListener(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void AXmsEntityRegistryListener::BeginPlay()
{
	Super::BeginPlay();

	if (UXmsRegistrySubsystem* RegistrySubsystem = UXmsRegistrySubsystem::Get(GetWorld()))
	{
		auto OnEntitiesCreatedDelegate = UXmsRegistrySubsystem::FOnEntityContextEvent::FDelegate::CreateUObject(this, &ThisClass::NativeOnEntitiesCreated);
		auto OnEntitiesDestroyedDelegate = UXmsRegistrySubsystem::FOnEntityContextEvent::FDelegate::CreateUObject(this, &ThisClass::NativeOnEntitiesDestroyed);

		OnEntitiesCreatedHandle = RegistrySubsystem->SubscribeToEntitiesCreated(MoveTemp(OnEntitiesCreatedDelegate));
		OnEntitiesDestroyedHandle = RegistrySubsystem->SubscribeToEntitiesDestroyed(MoveTemp(OnEntitiesDestroyedDelegate));

		UE_VLOG_UELOG(this, LogXms, Log, TEXT("%hs: Listening for XmsRegistry events"), __FUNCTION__);
	}
	else
	{
		UE_VLOG_UELOG(this, LogXms, Error, TEXT("%hs: UXmsRegistrySubsystem is required"), __FUNCTION__);
	}
}

void AXmsEntityRegistryListener::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UXmsRegistrySubsystem* RegistrySubsystem = UXmsRegistrySubsystem::Get(GetWorld()))
	{
		RegistrySubsystem->UnsubscribeFromEntitiesCreated(OnEntitiesCreatedHandle);
		RegistrySubsystem->UnsubscribeFromEntitiesDestroyed(OnEntitiesDestroyedHandle);
	}

	Super::EndPlay(EndPlayReason);
}

void AXmsEntityRegistryListener::NativeOnObservedEntitiesCreated(const EXmsEntityMetaType& MetaType, const TArray<UXmsRegistrySubsystem::FEntityContext>& EntityContexts)
{
	// Base class Empty
}

void AXmsEntityRegistryListener::NativeOnObservedEntitiesDestroyed(const EXmsEntityMetaType& MetaType, const TArray<UXmsRegistrySubsystem::FEntityContext>& EntityContexts)
{
	// Base class Empty
}

void AXmsEntityRegistryListener::NativeOnEntitiesCreated(const TArray<UXmsRegistrySubsystem::FEntityContext>& EntityContexts)
{
	checkSlow(EntityContexts.Num() > 0);
	const EXmsEntityMetaType& MetaType = EntityContexts[0].MetaData.MetaType;

	if (ObservedMetaTypes.Contains(MetaType))
	{
		NativeOnObservedEntitiesCreated(MetaType, EntityContexts);
	}
}

void AXmsEntityRegistryListener::NativeOnEntitiesDestroyed(const TArray<UXmsRegistrySubsystem::FEntityContext>& EntityContexts)
{
	checkSlow(EntityContexts.Num() > 0);
	const EXmsEntityMetaType& MetaType = EntityContexts[0].MetaData.MetaType;

	if (ObservedMetaTypes.Contains(MetaType))
	{
		NativeOnObservedEntitiesDestroyed(MetaType, EntityContexts);
	}
}
