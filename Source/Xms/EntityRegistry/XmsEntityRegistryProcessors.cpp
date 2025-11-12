// Copyright (c) 2025 Xist.GG

#include "EntityRegistry/XmsEntityRegistryProcessors.h"

#include "MassExecutionContext.h"
#include "XmsEntityMetaData.h"
#include "XmsEntityRegistry.h"
#include "XmsLog.h"

//----------------------------------------------------------------------//
//  UXmsEntityCreated
//----------------------------------------------------------------------//

UXmsEntityCreated::UXmsEntityCreated()
	: Query(*this)
{
	ObservedType = FXmsT_Registry::StaticStruct();
	ObservedOperations = EMassObservedOperationFlags::CreateEntity;
	ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::AllNetModes);
}

void UXmsEntityCreated::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	Query.AddTagRequirement<FXmsT_Registry>(EMassFragmentPresence::All);
	Query.AddConstSharedRequirement<FXmsCSF_MetaData>(EMassFragmentPresence::All);

	ProcessorRequirements.AddSubsystemRequirement<UXmsRegistrySubsystem>(EMassFragmentAccess::ReadWrite);
}

void UXmsEntityCreated::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	QUICK_SCOPE_CYCLE_COUNTER(UXmsEntityCreated);

	TArray<UXmsRegistrySubsystem::FEntityContext> Entities;
	Query.ForEachEntityChunk(Context,[&Entities](FMassExecutionContext& Context)
		{
			Entities.Reserve(Entities.Num() + Context.GetNumEntities());

			const auto& MetaData = Context.GetConstSharedFragment<FXmsCSF_MetaData>();

			for (FMassExecutionContext::FEntityIterator EntityIt = Context.CreateEntityIterator(); EntityIt; ++EntityIt)
			{
				UXmsRegistrySubsystem::FEntityContext EntityContext {
					.Entity = Context.GetEntity(*EntityIt),
					.MetaData = MetaData,  // COPY the MetaData
				};
				Entities.Emplace(EntityContext);
			}
	});

	if (Entities.Num() > 0)
	{
#if WITH_XMS_DEBUG
		UE_VLOG_UELOG(this, LogXmsRegistry, Verbose, TEXT("%hs: %i Entities Created"), __FUNCTION__, Entities.Num());
#endif

		UXmsRegistrySubsystem& RegistrySubsystem = Context.GetMutableSubsystemChecked<UXmsRegistrySubsystem>();
		RegistrySubsystem.MassOnEntitiesCreated(Entities);
	}
}

//----------------------------------------------------------------------//
//  UXmsEntityDestroyed
//----------------------------------------------------------------------//

UXmsEntityDestroyed::UXmsEntityDestroyed()
	: Query(*this)
{
	ObservedType = FXmsT_Registry::StaticStruct();
	ObservedOperations = EMassObservedOperationFlags::DestroyEntity;
	ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::AllNetModes);
}

void UXmsEntityDestroyed::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	Query.AddTagRequirement<FXmsT_Registry>(EMassFragmentPresence::All);
	Query.AddConstSharedRequirement<FXmsCSF_MetaData>(EMassFragmentPresence::All);

	ProcessorRequirements.AddSubsystemRequirement<UXmsRegistrySubsystem>(EMassFragmentAccess::ReadWrite);
}

void UXmsEntityDestroyed::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	QUICK_SCOPE_CYCLE_COUNTER(UXmsEntityDestroyed);

	TArray<UXmsRegistrySubsystem::FEntityContext> Entities;
	Query.ForEachEntityChunk(Context,[&Entities](FMassExecutionContext& Context)
		{
			Entities.Reserve(Entities.Num() + Context.GetNumEntities());

			const auto& MetaData = Context.GetConstSharedFragment<FXmsCSF_MetaData>();

			for (FMassExecutionContext::FEntityIterator EntityIt = Context.CreateEntityIterator(); EntityIt; ++EntityIt)
			{
				UXmsRegistrySubsystem::FEntityContext EntityContext {
					.Entity = Context.GetEntity(*EntityIt),
					.MetaData = MetaData,  // COPY the MetaData
				};
				Entities.Emplace(EntityContext);
			}
	});

	if (Entities.Num() > 0)
	{
#if WITH_XMS_DEBUG
		UE_VLOG_UELOG(this, LogXmsRegistry, Verbose, TEXT("%hs: %i Entities Destroyed"), __FUNCTION__, Entities.Num());
#endif

		UXmsRegistrySubsystem& RegistrySubsystem = Context.GetMutableSubsystemChecked<UXmsRegistrySubsystem>();
		RegistrySubsystem.MassOnEntitiesDestroyed(Entities);
	}
}
