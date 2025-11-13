// Copyright (c) 2025 Xist.GG

#include "Representation/XmsRepresentationProcessor.h"

#include "MassExecutionContext.h"
#include "XmsRepSubsystem.h"
#include "Attributes/Lifespan/XmsLifespan.h"
#include "EntityRegistry/XmsEntityMetaData.h"
#include "Misc/XmsFragments.h"

//----------------------------------------------------------------------//
//  UXimVizEntityAdded
//----------------------------------------------------------------------//

UXmsRepresentationProcessor::UXmsRepresentationProcessor()
	: Query(*this)
{
	ProcessingPhase = EMassProcessingPhase::FrameEnd;

	ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::All);
	ExecutionOrder.ExecuteInGroup = FName("Representation");
}

void UXmsRepresentationProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	Query.AddTagRequirement<FXmsT_Represent>(EMassFragmentPresence::All);
	Query.AddConstSharedRequirement<FXmsCSF_MetaData>(EMassFragmentPresence::All);
	Query.AddRequirement<FXmsF_Transform>(EMassFragmentAccess::ReadOnly);
	Query.AddRequirement<FXmsF_Lifespan>(EMassFragmentAccess::ReadOnly, EMassFragmentPresence::Optional);

	ProcessorRequirements.AddSubsystemRequirement<UXmsRepSubsystem>(EMassFragmentAccess::ReadWrite);
}

void UXmsRepresentationProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	QUICK_SCOPE_CYCLE_COUNTER(UXmsRepresentationProcessor);

	int32 TotalNumEntities = 0;
	TQueue<TArray<const FXmsEntityRepresentationData>> DataQueue;

	Query.ParallelForEachEntityChunk(Context, [&TotalNumEntities, &DataQueue](FMassExecutionContext& Context)
	{
		// MetaData is shared by all Entities in this chunk
		const auto& MetaData = Context.GetConstSharedFragment<FXmsCSF_MetaData>();

		// Every Entity has its own Transform
		const auto Transforms = Context.GetFragmentView<FXmsF_Transform>();

		// Some Entities have Lifespan, but it's optional.
		const auto Lifespans = Context.GetFragmentView<FXmsF_Lifespan>();
		const bool bHaveLifespan = Lifespans.Num() > 0;  // Do the Entities in this chunk have Lifespans?

		// Allocate memory to store data for the Game/Render threads
		TArray<const FXmsEntityRepresentationData> Entities;
		Entities.Reserve(Context.GetNumEntities());
		TotalNumEntities += Context.GetNumEntities();

		// Iterate over each Entity in this chunk, copy relevant info
		for (FMassExecutionContext::FEntityIterator EntityIt = Context.CreateEntityIterator(); EntityIt; ++EntityIt)
		{
			const FXmsF_Transform& Transform = Transforms[*EntityIt];

			// Any Entity without a Lifespan will report AlphaAge = -1
			// If this chunk has a lifespan, AlphaAge will be computed based on the Lifespan data.
			float AlphaAge {-1.};
			if (bHaveLifespan)
			{
				const FXmsF_Lifespan& Lifespan = Lifespans[*EntityIt];
				AlphaAge = Lifespan.MaxAge >= KINDA_SMALL_NUMBER && not Lifespan.IsImmortal()
					? Lifespan.CurrentAge / Lifespan.MaxAge
					: 1.;  // either Immortal or at max expected age
			}

			// This is the data that will be made available to the Game/Render systems
			const FXmsEntityRepresentationData Data {
				.Entity = Context.GetEntity(*EntityIt),
				.MetaType = MetaData.MetaType,
				.Location = Transform.Location,
				.AlphaAge = AlphaAge,
			};
			Entities.Emplace(Data);
		}

		// Thread-safe queue
		DataQueue.Enqueue(Entities);
	});

	// Update UXmsRepSubsystem data
	
	UXmsRepSubsystem& RepSubsystem = Context.GetMutableSubsystemChecked<UXmsRepSubsystem>();
	TArray<const FXmsEntityRepresentationData> Data;

	// Thread-safe rejoining of queue data
	RepSubsystem.MassPrepare(TotalNumEntities);
	while (DataQueue.Dequeue(OUT Data))
	{
		RepSubsystem.MassAppend(Data);
	}
	RepSubsystem.MassCommit();
}
