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
	Query.AddTagRequirement<FXmsT_Representation>(EMassFragmentPresence::All);
	Query.AddRequirement<FXmsF_Transform>(EMassFragmentAccess::ReadOnly);
	Query.AddRequirement<FXmsF_Lifespan>(EMassFragmentAccess::ReadOnly, EMassFragmentPresence::Optional);
	Query.AddConstSharedRequirement<FXmsCSF_MetaData>(EMassFragmentPresence::All);

	ProcessorRequirements.AddSubsystemRequirement<UXmsRepSubsystem>(EMassFragmentAccess::ReadWrite);
}

void UXmsRepresentationProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	QUICK_SCOPE_CYCLE_COUNTER(UXmsRepresentationProcessor);

	int32 TotalNumEntities = 0;
	TQueue<TArray<const FXmsEntityRepresentationData>> DataQueue;

	Query.ParallelForEachEntityChunk(Context, [&TotalNumEntities, &DataQueue](FMassExecutionContext& Context)
	{
		TotalNumEntities += Context.GetNumEntities();

		const auto& MetaData = Context.GetConstSharedFragment<FXmsCSF_MetaData>();
		const auto Lifespans = Context.GetFragmentView<FXmsF_Lifespan>();
		const auto Transforms = Context.GetFragmentView<FXmsF_Transform>();

		const bool bHaveLifespan = Lifespans.Num() > 0;

		TArray<const FXmsEntityRepresentationData> Entities;
		Entities.Reserve(Context.GetNumEntities());

		for (FMassExecutionContext::FEntityIterator EntityIt = Context.CreateEntityIterator(); EntityIt; ++EntityIt)
		{
			const FXmsF_Transform& Transform = Transforms[*EntityIt];

			float RelativeAge {-1.};
			if (bHaveLifespan)
			{
				const FXmsF_Lifespan& Lifespan = Lifespans[*EntityIt];
				RelativeAge = Lifespan.MaxAge >= KINDA_SMALL_NUMBER && not Lifespan.IsImmortal()
					? Lifespan.CurrentAge / Lifespan.MaxAge
					: 1.;  // either Immortal or at max expected age
			}

			const FXmsEntityRepresentationData Data {
				.Entity = Context.GetEntity(*EntityIt),
				.MetaType = MetaData.MetaType,
				.Location = Transform.Location,
				.Rotation = Transform.Rotation,
				.Scale3D = Transform.Scale3D,
				.AlphaAge = RelativeAge,
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
