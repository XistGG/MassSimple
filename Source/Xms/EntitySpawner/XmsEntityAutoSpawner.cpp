// Copyright (c) 2025 Xist.GG LLC

#include "XmsEntityAutoSpawner.h"

#include "MassEntityBuilder.h"
#include "MassEntityManager.h"
#include "MassEntityUtils.h"
#include "XmsLog.h"
#include "EntityRegistry/XmsEntityMetaData.h"

// Sets default values
AXmsEntityAutoSpawner::AXmsEntityAutoSpawner(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	SetHidden(true);

	EntityMetaType = EXmsEntityMetaType::None;
	SpawnIntervalSeconds = 1.;
}

void AXmsEntityAutoSpawner::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	TimeToNextSpawn -= DeltaSeconds;

	if (SpawnIntervalSeconds >= 0.
		&& TimeToNextSpawn <= 0.)
	{
		SpawnEntity();
		TimeToNextSpawn = SpawnIntervalSeconds;
	}
}

FMassEntityHandle AXmsEntityAutoSpawner::SpawnEntity()
{
#if WITH_XMS_DEBUG
	UE_VLOG_UELOG(this, LogXmsSpawner, Log, TEXT("%hs: %s: Spawning Entity"), __FUNCTION__, *GetClass()->GetName());
#endif

	const UWorld* World = GetWorld();
	check(World);

	FMassEntityManager& EntityManager = UE::Mass::Utils::GetEntityManagerChecked(*World);
	UE::Mass::FEntityBuilder Builder = EntityManager.MakeEntityBuilder();

	SetupEntityBuilder(OUT Builder);

	FMassEntityHandle ReservedEntity = Builder.GetEntityHandle();
	Builder.Commit();

	return ReservedEntity;
}

void AXmsEntityAutoSpawner::SetupEntityBuilder(UE::Mass::FEntityBuilder& Builder)
{
	// Set up Entity debug identification so we know which Class (or child class) created the Entity
	FMassArchetypeCreationParams ArchetypeCreationParams;
	ArchetypeCreationParams.DebugName = GetClass()->GetFName();
	Builder.ConfigureArchetypeCreation(ArchetypeCreationParams);

	// Set up Entity MetaData fragment
	FCSFXms_MetaData MetaData {
		.MetaType = EntityMetaType,
	};
	Builder.Add<FCSFXms_MetaData>(MetaData);

	// Required for observers?
	Builder.Add<FTXms_Register>();

#if WITH_XMS_DEBUG
	UE_CVLOG_UELOG(not MetaData.IsValid(), this, LogXmsSpawner, Warning,
		TEXT("%hs: %s: Invalid MetaData"), __FUNCTION__, *GetClass()->GetName());
#endif
}
