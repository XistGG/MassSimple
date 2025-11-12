// Copyright (c) 2025 Xist.GG

#include "XmsLifespanProcessors.h"

#include "MassExecutionContext.h"
#include "XmsLifespan.h"
#include "XmsLog.h"

//----------------------------------------------------------------------//
//  UXmsLifespanEnforcer
//----------------------------------------------------------------------//

UXmsLifespanEnforcer::UXmsLifespanEnforcer()
	: Query(*this)
{
	ProcessingPhase = EMassProcessingPhase::FrameEnd;

	ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::All);
	ExecutionOrder.ExecuteInGroup = FName("Lifespan");
	// Kill entities for next frame before representing them for next frame
	ExecutionOrder.ExecuteBefore.Add(FName("Representation"));
}

void UXmsLifespanEnforcer::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	Query.AddRequirement<FXmsF_Lifespan>(EMassFragmentAccess::ReadWrite);
}

void UXmsLifespanEnforcer::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	QUICK_SCOPE_CYCLE_COUNTER(UXmsLifespanEnforcer);

	Query.ParallelForEachEntityChunk(Context, [LogObject=this](FMassExecutionContext& Context)
	{
		TArray<FMassEntityHandle> EntitiesToDestroy;

		const auto Lifespans = Context.GetMutableFragmentView<FXmsF_Lifespan>();

		for (FMassExecutionContext::FEntityIterator EntityIt = Context.CreateEntityIterator(); EntityIt; ++EntityIt)
		{
			FXmsF_Lifespan& Lifespan = Lifespans[*EntityIt];

			Lifespan.CurrentAge += Context.GetDeltaTimeSeconds();

			if (Lifespan.CurrentAge > Lifespan.MaxAge
				&& not Lifespan.IsImmortal())
			{
				EntitiesToDestroy.Add(Context.GetEntity(*EntityIt));
			}
		}

		// Schedule the destruction of Entities that are too old
		if (EntitiesToDestroy.Num() > 0)
		{
#if WITH_XMS_DEBUG
			UE_VLOG_UELOG(LogObject, LogXms, Verbose, TEXT("%hs: Destroy %i Entities due to old age"),
				__FUNCTION__, EntitiesToDestroy.Num());
#endif

			Context.Defer().DestroyEntities(EntitiesToDestroy);
		}
	});
}
