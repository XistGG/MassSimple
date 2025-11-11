// Copyright (c) 2025 Xist.GG LLC

#include "XmsEntityTreeBuilder.h"

#include "MassEntityBuilder.h"
#include "MassEntityManager.h"
#include "MassEntityUtils.h"
#include "XmsLog.h"
#include "EntityRegistry/XmsEntityMetaData.h"
#include "Misc/XmsFragments.h"
#include "Representation/XmsRepSubsystem.h"

// Sets default values
AXmsEntityTreeBuilder::AXmsEntityTreeBuilder(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	SetHidden(true);

	BuildWorldZ = 0.;

	EntityMetaType = EXmsEntityMetaType::Tree;
	EntityLifespan = FXmsF_Lifespan {
		.MaxAge = 60.,
	};
	bAutoBuildEnabled = true;
	AutoBuildIntervalSeconds = 1.;
}

void AXmsEntityTreeBuilder::Tick(float DeltaSeconds)
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

FMassEntityHandle AXmsEntityTreeBuilder::BuildEntity()
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

void AXmsEntityTreeBuilder::SetupEntityBuilder(UE::Mass::FEntityBuilder& Builder)
{
	// Set up Entity MetaData fragment
	FXmsCSF_MetaData MetaData {
		.MetaType = EntityMetaType,
	};

	// Warn if/when the MetaData is invalid
	UE_CVLOG_UELOG(not MetaData.IsValid(), this, LogXmsBuilder, Warning,
		TEXT("%hs: %s: Invalid MetaData for reserved Entity [%s]"),
		__FUNCTION__, *GetClass()->GetName(), *Builder.GetEntityHandle().DebugGetDescription());

	// Set the transform for the Entity
	FXmsF_Transform EntityTransform;
	InitEntityTransform(OUT EntityTransform);

	// Configure Builder

	Builder.Add<FXmsCSF_MetaData>(MetaData);
	Builder.Add<FXmsF_Lifespan>(EntityLifespan);
	Builder.Add<FXmsF_Transform>(EntityTransform);

	// For now, tag all Entities as being visible
	Builder.Add<FXmsT_Representation>();

	// Mass Observers apparently cannot observe Const Shared Fragments for created Entities,
	// so we will use this Tag for Registry Observer compatibility.
	Builder.Add<FXmsT_Registry>();
}

void AXmsEntityTreeBuilder::InitEntityTransform(FXmsF_Transform& OutTransform)
{
	const FBox BoundsBox = GetBounds().GetBox();

	const FVector Center = BoundsBox.GetCenter();
	const FVector Extent = BoundsBox.GetExtent();

	const FVector MinPoint = Center - Extent;
	const FVector MaxPoint = Center + Extent;

	const double X = FMath::RandRange(MinPoint.X, MaxPoint.X);
	const double Y = FMath::RandRange(MinPoint.Y, MaxPoint.Y);
	const double Z = BuildWorldZ;

	const double Yaw = FMath::RandRange(0., 360.);

	OutTransform.Location = FVector(X, Y, Z);
	OutTransform.Rotation = FRotator(0., Yaw, 0.);
	OutTransform.Scale3D = FVector::OneVector;
}
