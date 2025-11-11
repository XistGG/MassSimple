// Copyright (c) 2025 Xist.GG LLC

#include "XmsRepSubsystem.h"

#include "CanvasItem.h"
#include "CanvasTypes.h"
#include "EngineUtils.h"
#include "GlobalRenderResources.h"
#include "Engine/Canvas.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/World.h"
#include "Kismet/KismetRenderingLibrary.h"

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

	CanvasScale = 1.;
	WorldPlaneName = FName("WorldPlane");

	WorldOrigin = FVector::ZeroVector;
	WorldExtent = FVector::OneVector;
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

	for (TActorIterator<AStaticMeshActor> It(&InWorld); It; ++It)
	{
		AStaticMeshActor* Actor = *It;
		if (Actor->GetName() == WorldPlaneName)
		{
			WorldPlaneActor = Actor;
		}
	}

	if (ensureMsgf(not SoftRenderTarget.IsNull(),
		TEXT("SoftRenderTarget is expected to be configured in INI")))
	{
		if (RenderTarget = Cast<UTextureRenderTarget2D>(SoftRenderTarget.ToSoftObjectPath().TryLoad());
			ensureMsgf(RenderTarget,
				TEXT("SoftRenderTarget path load failed: %s"), *SoftRenderTarget.ToSoftObjectPath().ToString()))
		{
			if (WorldPlaneActor)
			{
				constexpr bool bOnlyCollidingComponents = true;
				constexpr bool bIncludeFromChildActors = false;
				WorldPlaneActor->GetActorBounds(bOnlyCollidingComponents, OUT WorldOrigin, OUT WorldExtent, bIncludeFromChildActors);

				CanvasSize.X = FMath::CeilToInt32(CanvasScale * 2. * WorldExtent.X);
				CanvasSize.Y = FMath::CeilToInt32(CanvasScale * 2. * WorldExtent.Y);

				RenderTarget->ResizeTarget(CanvasSize.X, CanvasSize.Y);
			}

			constexpr bool bClearRenderTarget = true;
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
}

void UXmsRepSubsystem::UpdateEntities()
{
	QUICK_SCOPE_CYCLE_COUNTER(UXmsRepSubsystem_UpdateEntities);

	if (RenderTarget == nullptr)
	{
		return;
	}

	int32 CurrentPage;
	{
		UE::TReadScopeLock ReadLock (EntityDataPageLock);
		CurrentPage = EntityDataCurrentPage;  // COPY the data
	}

	TArray<const FXmsEntityRepresentationData>* CurrentData = &EntityDataPages[CurrentPage];

	UKismetRenderingLibrary::ClearRenderTarget2D(this, RenderTarget, FLinearColor::Transparent);

	UCanvas* Canvas {nullptr};
	FVector2D Size;
	FDrawToRenderTargetContext Context;
	UKismetRenderingLibrary::BeginDrawCanvasToRenderTarget(this, RenderTarget, OUT Canvas, OUT Size, OUT Context);

	for (const FXmsEntityRepresentationData& Data : *CurrentData)
	{
		FVector2D EntitySize (1.);
		FLinearColor EntityColor (FLinearColor::White);

		switch (Data.MetaType)
		{
		case EXmsEntityMetaType::Tree:
			EntityColor = FLinearColor::Green;
			EntitySize *= 1.5;
			break;
		case EXmsEntityMetaType::Wisp:
			EntityColor = FLinearColor::Yellow;
			break;
		default: break;
		}

		const FVector2D EntityOffset (FVector2D(CanvasSize) / EntitySize / 2.);
		const FVector2D CanvasLocation = WorldToCanvas(Data.Location);
		const FVector2D CanvasPosition (
			FMath::Clamp(CanvasLocation.X - EntityOffset.X, 0., CanvasSize.X),
			FMath::Clamp(CanvasLocation.Y - EntityOffset.Y, 0., CanvasSize.Y));

		FCanvasTileItem TileItem (CanvasPosition, GWhiteTexture, EntitySize, EntityColor);
		TileItem.BlendMode = FCanvas::BlendToSimpleElementBlend(BLEND_Opaque);
		Canvas->DrawItem(TileItem);
	}

	UKismetRenderingLibrary::EndDrawCanvasToRenderTarget(this, Context);
}

FVector2D UXmsRepSubsystem::WorldToCanvas(const FVector& Location) const
{
	const FVector RelativeLocation = Location - WorldOrigin;

	const float AlphaX = FMath::Abs(RelativeLocation.X) < UE_KINDA_SMALL_NUMBER ? 0. : 2. * WorldExtent.X / RelativeLocation.X;
	const float AlphaY = FMath::Abs(RelativeLocation.Y) < UE_KINDA_SMALL_NUMBER ? 0. : 2. * WorldExtent.Y / RelativeLocation.Y;

	const FVector2D Result (AlphaX * CanvasSize.X, AlphaY * CanvasSize.Y);
	return Result;
}
