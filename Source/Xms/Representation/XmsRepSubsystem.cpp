// Copyright (c) 2025 Xist.GG LLC

#include "XmsRepSubsystem.h"

#include "CanvasItem.h"
#include "CanvasTypes.h"
#include "EngineUtils.h"
#include "GlobalRenderResources.h"
#include "XmsLog.h"
#include "Engine/Canvas.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/World.h"
#include "Kismet/KismetRenderingLibrary.h"

#define WITH_XMS_REPRESENTATION_DEBUG       (true && WITH_XMS_DEBUG)
#define WITH_XMS_REPRESENTATION_DRAW_DEBUG  (false && WITH_XMS_REPRESENTATION_DEBUG)

#if WITH_XMS_REPRESENTATION_DRAW_DEBUG
#include "DrawDebugHelpers.h"
#endif

//----------------------------------------------------------------------//
//  UXmsRepSubsystem
//----------------------------------------------------------------------//

UXmsRepSubsystem* UXmsRepSubsystem::Get(TNotNull<const UWorld*> World)
{
	UXmsRepSubsystem* Result = World->GetSubsystem<UXmsRepSubsystem>();
	return Result;
}

UXmsRepSubsystem& UXmsRepSubsystem::GetChecked(TNotNull<const UWorld*> World)
{
	UXmsRepSubsystem* Result = Get(World);
	check(Result);
	return *Result;
}

UXmsRepSubsystem::UXmsRepSubsystem()
{
	EntityDataCurrentPage = 0;
	EntityDataTempPage = 1;

	CanvasPixelWorldSize = 100.;  // 1px == 1m
	WorldPlaneName = FName("WorldPlane");

	WorldOrigin = FVector::ZeroVector;
	WorldExtent = FVector::OneVector;  // MUST BE NONZERO
	CanvasSize = FIntVector2(1);
}

void UXmsRepSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UXmsRepSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

void UXmsRepSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	// NOTICE: If you are using this UXmsRepSubsystem in a GameFeature plugin,
	// then DO NOT DO STUFF ON BeginPlay! INSTEAD, WAIT FOR YOUR GFP TO LOAD.
	// In Lyra for example, wait for the Lyra OnExperienceLoaded event.

	// In this example project, we aren't using GFPs. We can access assets
	// as soon as the world has loaded.

	// Find the WorldPlane actor in the world.
	// We expect a static mesh actor that contains the "ground" of the world.

	WorldPlaneActor = FindWorldPlane(InWorld);

	if (not ensureMsgf(WorldPlaneActor != nullptr, TEXT("Cannot find WorldPlane Actor")))
	{
		return;
	}

	// Given the world ground plane, compute the world origin and extent.

	constexpr bool bOnlyCollidingComponents = true;
	constexpr bool bIncludeFromChildActors = false;
	WorldPlaneActor->GetActorBounds(bOnlyCollidingComponents, OUT WorldOrigin, OUT WorldExtent, bIncludeFromChildActors);

	// Make SURE WorldExtent components are never zeroish!
	WorldExtent.X = FMath::Max(1., WorldExtent.X);
	WorldExtent.Y = FMath::Max(1., WorldExtent.Y);
	WorldExtent.Z = FMath::Max(1., WorldExtent.Z);

	// WorldSize components are also guaranteed to never be zeroish
	WorldSize = 2. * WorldExtent;

	// Deduce the size of the RenderTarget canvas based on its desired world scale
	// and the size of our world plane.

	CanvasPixelWorldSize = FMath::Max(KINDA_SMALL_NUMBER, CanvasPixelWorldSize);
	const float CanvasScale = 1. / CanvasPixelWorldSize;

	CanvasSize.X = FMath::CeilToInt32(CanvasScale * WorldSize.X);
	CanvasSize.Y = FMath::CeilToInt32(CanvasScale * WorldSize.Y);

	UE_VLOG_UELOG(this, LogXmsRepresentation, Display,
		TEXT("%hs: Initializing RenderTarget size [%i, %i] for World size [%0.0f, %0.0f]"),
		__FUNCTION__, CanvasSize.X, CanvasSize.Y, WorldSize.X, WorldSize.Y);

	// Load and initialize the RenderTarget

	if (ensureMsgf(not SoftRenderTarget.IsNull(),
		TEXT("SoftRenderTarget is expected to be configured in INI")))
	{
		if (RenderTarget = Cast<UTextureRenderTarget2D>(SoftRenderTarget.ToSoftObjectPath().TryLoad());
			ensureMsgf(RenderTarget, TEXT("SoftRenderTarget path failed to load: %s"),
				*SoftRenderTarget.ToSoftObjectPath().ToString()))
		{
			constexpr bool bClearRenderTarget = true;

			RenderTarget->ResizeTarget(CanvasSize.X, CanvasSize.Y);
			RenderTarget->UpdateResourceImmediate(bClearRenderTarget);
		}
	}
}

void UXmsRepSubsystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateEntities();
}

TStatId UXmsRepSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UXmsRepSubsystem, STATGROUP_Tickables);
}

void UXmsRepSubsystem::MassPrepare(const int32 ExpectedNum)
{
	EntityDataPages[EntityDataTempPage].Reset(ExpectedNum);
}

void UXmsRepSubsystem::MassAppend(const TArray<const FXmsEntityRepresentationData>& Entities)
{
	EntityDataPages[EntityDataTempPage].Append(Entities);
}

void UXmsRepSubsystem::MassCommit()
{
	QUICK_SCOPE_CYCLE_COUNTER(UXmsRepSubsystem_Commit);

	{
		UE::TWriteScopeLock WriteLock (EntityDataPageLock);

		EntityDataCurrentPage = EntityDataTempPage;
		if (++EntityDataTempPage >= EntityDataPages.Num())
		{
			EntityDataTempPage = 0;
		}
	}

#if WITH_XMS_REPRESENTATION_DEBUG
	UE_VLOG_UELOG(this, LogXmsRepresentation, Verbose,
		TEXT("%llu: %hs: Ingested Mass data to page=%i"),
		GFrameCounter, __FUNCTION__, EntityDataCurrentPage);
#endif
}

void UXmsRepSubsystem::UpdateEntities()
{
	QUICK_SCOPE_CYCLE_COUNTER(UXmsRepSubsystem_UpdateEntities);

	if (not ensure(RenderTarget))
	{
		return;
	}

	int32 CurrentPage;
	{
		UE::TReadScopeLock ReadLock (EntityDataPageLock);
		CurrentPage = EntityDataCurrentPage;
	}

	check(CurrentPage >= 0 && CurrentPage < EntityDataPages.Num());
	TArray<const FXmsEntityRepresentationData>* CurrentDataPagePtr = &EntityDataPages[CurrentPage];

	UKismetRenderingLibrary::ClearRenderTarget2D(this, RenderTarget, FLinearColor::Transparent);

	UCanvas* Canvas {nullptr};
	FVector2D Size;
	FDrawToRenderTargetContext Context;
	UKismetRenderingLibrary::BeginDrawCanvasToRenderTarget(this, RenderTarget, OUT Canvas, OUT Size, OUT Context);

	constexpr double MaxTreeSize = 10.;

	for (const FXmsEntityRepresentationData& Data : *CurrentDataPagePtr)
	{
		FLinearColor EntityColor (FLinearColor::White);
		double SizeScale {1.};

		switch (Data.MetaType)
		{
		case EXmsEntityMetaType::Tree:
			EntityColor = FLinearColor::Green;
			SizeScale = FMath::Clamp(Data.AlphaAge * MaxTreeSize, 1., MaxTreeSize);
			break;
		case EXmsEntityMetaType::Wisp:
			EntityColor = FLinearColor::Yellow;
			break;
		default: break;
		}

		EntityColor.A = 0.6;

		const FVector2D EntitySize (SizeScale);
		const FVector2D EntityHalfSize (EntitySize / 2.);

#if WITH_XMS_REPRESENTATION_DRAW_DEBUG
		// Draw a Debug Sphere at this Entity's world location
		DrawDebugSphere(GetWorld(), Data.Location, 50.0f, 12, FColor::Yellow, true, 5.);
#endif

		const FVector2D CanvasPoint = WorldToCanvas(Data.Location);
		const FVector2D CenteredCanvasPoint (
			FMath::Clamp(CanvasPoint.X - EntityHalfSize.X, 0., CanvasSize.X),
			FMath::Clamp(CanvasPoint.Y - EntityHalfSize.Y, 0., CanvasSize.Y)
			);

		FCanvasTileItem TileItem (CenteredCanvasPoint, GWhiteTexture, EntitySize, EntityColor);
		TileItem.BlendMode = FCanvas::BlendToSimpleElementBlend(EBlendMode::BLEND_Translucent);
		TileItem.PivotPoint = FVector2D::ZeroVector;
		TileItem.Rotation = FRotator::ZeroRotator;
		Canvas->DrawItem(TileItem);
	}

	UKismetRenderingLibrary::EndDrawCanvasToRenderTarget(this, Context);

#if WITH_XMS_REPRESENTATION_DEBUG
	UE_VLOG_UELOG(this, LogXmsRepresentation, Verbose,
		TEXT("%llu: %hs: Drew %i Entities (page=%i) to RenderTarget"),
		GFrameCounter, __FUNCTION__, CurrentDataPagePtr->Num(), CurrentPage);
#endif
}

FVector2D UXmsRepSubsystem::WorldToCanvas(const FVector& Location) const
{
	const FVector WorldMin = WorldOrigin - WorldExtent;
	const FVector WorldMax = WorldOrigin + WorldExtent;

	const FVector SafeLoc (
		FMath::Clamp(Location.X, WorldMin.X, WorldMax.X),
		FMath::Clamp(Location.Y, WorldMin.Y, WorldMax.Y),
		WorldOrigin.Z  // unused
		);
	const FVector RelativeLoc = SafeLoc - WorldMin;
	const FVector Alpha = RelativeLoc / WorldSize;  // WorldSize components are guaranteed non-zeroish

	const FVector2D Result (
		FMath::Min(1., Alpha.X) * CanvasSize.X,
		FMath::Min(1., Alpha.Y) * CanvasSize.Y
		);
	return Result;
}

AStaticMeshActor* UXmsRepSubsystem::FindWorldPlane(const UWorld& World) const
{
#if UE_BUILD_DEVELOPMENT || UE_BUILD_DEBUG

	// NOTICE: THIS ACTOR SEARCH METHODOLOGY DOES NOT WORK FOR A SHIPPING GAME.
	// The Actor Label is only available in Development+Debug configurations.
	// There are many ways to solve this that are beyond the scope of this project.

	for (TActorIterator<AStaticMeshActor> It(&World); It; ++It)
	{
		AStaticMeshActor* Actor = *It;
		const FString Label = Actor->GetActorLabel();

		if (Label == WorldPlaneName)
		{
			return Actor;
		}
	}

#endif
	return nullptr;
}

#undef WITH_XMS_REPRESENTATION_DRAW_DEBUG
#undef WITH_XMS_REPRESENTATION_DEBUG
