// Copyright (c) 2025 Xist.GG

#include "XmsLifespanProcessors.h"

#include "MassExecutionContext.h"
#include "XmsLifespan.h"

UXmsLifespanEnforcer::UXmsLifespanEnforcer()
	: Query(*this)
{
	ProcessingPhase = EMassProcessingPhase::FrameEnd;

	ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::All);
	ExecutionOrder.ExecuteInGroup = FName("Lifespan");
}

void UXmsLifespanEnforcer::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	Query.AddRequirement<FXmsF_Lifespan>(EMassFragmentAccess::ReadWrite);
}

void UXmsLifespanEnforcer::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	QUICK_SCOPE_CYCLE_COUNTER(UXmsLifespanEnforcer);

	Query.ParallelForEachEntityChunk(Context, [](FMassExecutionContext& Context)
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
			Context.Defer().DestroyEntities(EntitiesToDestroy);
		}
	});
}
