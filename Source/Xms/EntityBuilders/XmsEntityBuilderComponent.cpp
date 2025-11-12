// Copyright (c) 2025 Xist.GG LLC

#include "XmsEntityBuilderComponent.h"

#include "MassEntityBuilder.h"
#include "MassEntityManager.h"
#include "MassEntityUtils.h"
#include "XmsLog.h"
#include "EntityRegistry/XmsEntityRegistry.h"
#include "GameFramework/Actor.h"
#include "Misc/XmsFragments.h"
#include "Representation/XmsRepSubsystem.h"

// Set Class Defaults
UXmsEntityBuilderComponent::UXmsEntityBuilderComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;

	EntityMetaType = EXmsEntityMetaType::Wisp;
	EntityLifespan = FXmsF_Lifespan {
		.MaxAge = 4.,
	};
	bAutoBuildEnabled = true;
	AutoBuildIntervalSeconds = 0.25;
}

void UXmsEntityBuilderComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// If it's time to auto-build a new Entity, then do it now.

	TimeToNextAutoBuild -= DeltaTime;

	if (IsAutoBuildEnabled()
		&& TimeToNextAutoBuild <= 0.)
	{
		BuildEntity();
		TimeToNextAutoBuild = AutoBuildIntervalSeconds;
	}
}

FMassEntityHandle UXmsEntityBuilderComponent::BuildEntity()
{
	UE_VLOG_UELOG(this, LogXmsBuilder, Verbose, TEXT("%hs: %s: Building Entity"),
		__FUNCTION__, *GetClass()->GetName());

	const UWorld* World = GetWorld();
	check(World);

	FMassEntityManager& EntityManager = UE::Mass::Utils::GetEntityManagerChecked(*World);
	UE::Mass::FEntityBuilder Builder = EntityManager.MakeEntityBuilder();

	// Set up Entity debug identification so we know which child class created any given Entity
	FMassArchetypeCreationParams ArchetypeCreationParams;
	ArchetypeCreationParams.DebugName = GetClass()->GetFName();
	Builder.ConfigureArchetypeCreation(ArchetypeCreationParams);

	// Allow child classes to customize Builder setup
	SetupEntityBuilder(OUT Builder);

	const FMassEntityHandle Entity = Builder.Commit();
	return Entity;
}

void UXmsEntityBuilderComponent::SetupEntityBuilder(UE::Mass::FEntityBuilder& Builder)
{
	const AActor* OwnerActor = GetOwner();
	check(OwnerActor);

	// Set up Entity MetaData fragment
	FXmsCSF_MetaData MetaData {
		.MetaType = EntityMetaType,
	};

	// Warn if/when the MetaData is invalid
	UE_CVLOG_UELOG(not MetaData.IsValid(), this, LogXmsBuilder, Warning,
		TEXT("%hs: %s: Invalid MetaData for reserved Entity [%s]"),
		__FUNCTION__, *GetClass()->GetName(), *Builder.GetEntityHandle().DebugGetDescription());

	// Spawn the Entity with a copy of the owner actor's current transform
	FXmsF_Transform Transform {
		.Location = OwnerActor->GetActorLocation(),
		.Rotation = OwnerActor->GetActorRotation(),
		.Scale3D = OwnerActor->GetActorScale(),
	};

	// Configure Builder

	Builder.Add<FXmsCSF_MetaData>(MetaData);
	Builder.Add<FXmsF_Lifespan>(EntityLifespan);
	Builder.Add<FXmsF_Transform>(Transform);

	// For now, tag all Entities as being visible
	Builder.Add<FXmsT_Represent>();

	// Mass Observers apparently cannot observe Const Shared Fragments for created Entities,
	// so we will use this Tag for Registry Observer compatibility.
	Builder.Add<FXmsT_Registry>();
}
