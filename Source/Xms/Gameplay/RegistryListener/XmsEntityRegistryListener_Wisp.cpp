// Copyright (c) 2025 Xist.GG LLC

#include "XmsEntityRegistryListener_Wisp.h"

#include "MassEntityUtils.h"
#include "MassEntityView.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "XmsLog.h"
#include "Common/XmsFragments.h"

// Set Class Defaults
AXmsEntityRegistryListener_Wisp::AXmsEntityRegistryListener_Wisp(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ObservedMetaTypes = {EXmsEntityMetaType::Wisp};
}

void AXmsEntityRegistryListener_Wisp::BeginPlay()
{
	Super::BeginPlay();

	if (not NiagaraSystemPath.IsEmpty())
	{
		NiagaraSystem = Cast<UNiagaraSystem>(StaticLoadObject(UNiagaraSystem::StaticClass(), nullptr,
			NiagaraSystemPath, nullptr, LOAD_None, nullptr));
	}

	UE_CLOG(NiagaraSystem == nullptr, LogXms, Error, TEXT("%hs: NiagaraSystem [%s] is not valid"),
		__FUNCTION__, *NiagaraSystemPath);
}

void AXmsEntityRegistryListener_Wisp::NativeOnObservedEntitiesCreated(const EXmsEntityMetaType& MetaType, const TArray<UXmsRegistrySubsystem::FEntityContext>& EntityContexts)
{
	// DO NOT CALL PURE VIRTUAL SUPER
	//-Super::NativeOnObservedEntitiesCreated(MetaType, EntityContexts);

	if (NiagaraSystem)
	{
		const UWorld* World = GetWorld();
		check(World);

		FMassEntityManager& EntityManager = UE::Mass::Utils::GetEntityManagerChecked(*World);

		for (const UXmsRegistrySubsystem::FEntityContext& Context : EntityContexts)
		{
			FMassEntityView EntityView (EntityManager, Context.Entity);
			const FXmsF_Transform& Transform = EntityView.GetFragmentData<FXmsF_Transform>();

			UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, NiagaraSystem,
				Transform.Location, Transform.Rotation, Transform.Scale3D);
		}
	}
}

void AXmsEntityRegistryListener_Wisp::NativeOnObservedEntitiesDestroyed(const EXmsEntityMetaType& MetaType, const TArray<UXmsRegistrySubsystem::FEntityContext>& EntityContexts)
{
	// DO NOT CALL PURE VIRTUAL SUPER
	//-Super::NativeOnObservedEntitiesDestroyed(MetaType, EntityContexts);

	// You could for example remember all the things you spawned, and now despawn them
	// or otherwise clean them up.

	// In our case, the Niagara Systems despawn themselves after they execute a single animation loop.
}
