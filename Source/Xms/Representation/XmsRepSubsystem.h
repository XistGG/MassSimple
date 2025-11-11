// Copyright (c) 2025 Xist.GG LLC

#pragma once

#include "MassSubsystemBase.h"
#include "EntityRegistry/XmsEntityMetaData.h"
#include "Misc/TransactionallySafeRWLock.h"

#include "XmsRepSubsystem.generated.h"

class AStaticMeshActor;
class UTextureRenderTarget2D;

/**
 * FXmsT_Represent
 *
 * Used as a filter by UXmsRepresentationProcessor
 *
 * Every Entity should be represented needs to have this tag.
 */
USTRUCT()
struct FXmsT_Represent
	: public FMassTag
{
	GENERATED_BODY()
};

/**
 * FXmsEntityRepresentationData
 *
 * This is the data that is copied from Mass->Game by the UXmsRepresentationProcessor.
 * Add/modify this as needed to extract the data required.
 */
USTRUCT()
struct FXmsEntityRepresentationData
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere)
	FMassEntityHandle Entity;

	UPROPERTY(VisibleAnywhere)
	EXmsEntityMetaType MetaType = EXmsEntityMetaType::None;

	UPROPERTY(VisibleAnywhere)
	FVector Location = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere)
	float AlphaAge = -1.;
};

/**
 * UXmsRepSubsystem
 *
 * This subsystem makes the assumption that ONLY ONE Mass Processor will be
 * calling into its Mass* methods. In that case, it is thread-safe.
 */
UCLASS(Config=Xms)
class XMS_API UXmsRepSubsystem
	: public UMassTickableSubsystemBase
{
	GENERATED_BODY()

public:
	static UXmsRepSubsystem* Get(TNotNull<const UWorld*> World);
	static UXmsRepSubsystem& GetChecked(TNotNull<const UWorld*> World);

	// Set Class Defaults
	UXmsRepSubsystem();

	//~Begin UWorldSubsystem interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	//~End UWorldSubsystem interface

	//~Begin UTickableWorldSubsystem interface
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;
	//~End UTickableWorldSubsystem interface

	/**
	 * Prepare for a bunch of incoming MassAppend().
	 * @param ExpectedNum Total number of Entities you expect to receive from Egress calls
	 */
	void MassPrepare(const int32 ExpectedNum);

	/**
	 * Append a subset of Entities to the representation system
	 * @param Entities A subset of Entities to represent
	 */
	void MassAppend(const TArray<const FXmsEntityRepresentationData>& Entities);

	/**
	 * Save all the Entities accumulated since MassPrepare()
	 */
	void MassCommit();

protected:
	/**
	 * Number of square cm any given Canvas Pixel should represent (must be >= KINDA_SMALL_NUMBER)
	 */
	UPROPERTY(Config, meta=(ForceUnits="cm", ClampMin=0.0001))
	float CanvasPixelWorldSize;

	/**
	 * Soft object path to the RenderTarget this subsystem will use for visualization
	 */
	UPROPERTY(Config)
	TSoftObjectPtr<UTextureRenderTarget2D> SoftRenderTarget;

	/**
	 * Name of AStaticMeshActor in the world that will give us the world bounds.
	 * This actor MUST exist in the level or this subsystem will not function.
	 */
	UPROPERTY(Config)
	FName WorldPlaneName;

	/**
	 * Called during Tick to draw visible Entities to the RenderTarget
	 */
	void UpdateEntities();

	/**
	 * Convert a World Location to a Canvas Point
	 * @param Location The World Location of interest
	 * @return Canvas Point representing the World Location
	 */
	FVector2D WorldToCanvas(const FVector& Location) const;

private:
	UPROPERTY(Transient)
	FIntVector2 CanvasSize;

	UPROPERTY(Transient)
	FVector WorldOrigin;

	UPROPERTY(Transient)
	FVector WorldExtent;

	UPROPERTY(Transient)
	FVector WorldSize;

	UPROPERTY(Transient)
	TObjectPtr<UTextureRenderTarget2D> RenderTarget;

	UPROPERTY(Transient)
	TObjectPtr<AStaticMeshActor> WorldPlaneActor;

	TStaticArray<TArray<const FXmsEntityRepresentationData>, 3> EntityDataPages;
	FTransactionallySafeRWLock EntityDataPageLock;
	int32 EntityDataCurrentPage;
	int32 EntityDataTempPage;

	/**
	 * Iterate World's Actors, looking for the one we expect is the WorldPlane
	 * @param World The World in which to search
	 * @return nullptr, or the WorldPlane actor if found
	 */
	AStaticMeshActor* FindWorldPlane(const UWorld& World) const;
};

template<>
struct TMassExternalSubsystemTraits<UXmsRepSubsystem> final
{
	enum
	{
		GameThreadOnly = false,  // YES read thread-safe
		ThreadSafeWrite = true,  // YES write thread-safe
	};
};
