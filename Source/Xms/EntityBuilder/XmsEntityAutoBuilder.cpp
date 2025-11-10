// Copyright (c) 2025 Xist.GG LLC

#include "XmsEntityAutoBuilder.h"

#include "MassEntityBuilder.h"
#include "MassEntityManager.h"
#include "MassEntityUtils.h"
#include "XmsLog.h"
#include "EntityRegistry/XmsEntityMetaData.h"

// Sets default values
AXmsEntityAutoBuilder::AXmsEntityAutoBuilder(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	SetHidden(true);

	EntityMetaType = EXmsEntityMetaType::None;
	AutoBuildIntervalSeconds = 1.;
}

void AXmsEntityAutoBuilder::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// If it's time to auto-build a new Entity, then do it now.

	TimeToNextAutoBuild -= DeltaSeconds;

	if (IsAutoBuildEnabled()
		&& TimeToNextAutoBuild <= 0.)
	{
		BuildEntity();
		TimeToNextAutoBuild = AutoBuildIntervalSeconds;
	}
}

FMassEntityHandle AXmsEntityAutoBuilder::BuildEntity()
{
	UE_VLOG_UELOG(this, LogXmsBuilder, Verbose, TEXT("%hs: %s: Building Entity"),
		__FUNCTION__, *GetClass()->GetName());

	const UWorld* World = GetWorld();
	check(World);

	FMassEntityManager& EntityManager = UE::Mass::Utils::GetEntityManagerChecked(*World);
	UE::Mass::FEntityBuilder Builder = EntityManager.MakeEntityBuilder();

	SetupEntityBuilder(OUT Builder);

	const FMassEntityHandle Entity = Builder.Commit();
	return Entity;
}

void AXmsEntityAutoBuilder::SetupEntityBuilder(UE::Mass::FEntityBuilder& Builder)
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

	// Mass Observers apparently cannot observe Const Shared Fragments for created Entities,
	// so we will use this Tag for Observer compatibility.
	Builder.Add<FTXms_Registry>();

	UE_CVLOG_UELOG(not MetaData.IsValid(), this, LogXmsBuilder, Warning,
		TEXT("%hs: %s: Invalid MetaData for reserved Entity [%s]"),
		__FUNCTION__, *GetClass()->GetName(), *Builder.GetEntityHandle().DebugGetDescription());
}
