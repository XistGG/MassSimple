// Copyright (c) 2025 Xist.GG LLC

#pragma once

#include "MassSubsystemBase.h"
#include "EntityRegistry/XmsEntityMetaData.h"
#include "Misc/TransactionallySafeRWLock.h"

#include "XmsRepSubsystem.generated.h"

class AStaticMeshActor;
class UTextureRenderTarget2D;

/**
 * FXmsT_Representation
 */
USTRUCT()
struct FXmsT_Representation
	: public FMassTag
{
	GENERATED_BODY()
};

/**
 * FXmsEntityRepresentationData
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
	FRotator Rotation = FRotator::ZeroRotator;

	UPROPERTY(VisibleAnywhere)
	FVector Scale3D = FVector::OneVector;
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
	 * 
	 */
	void UpdateEntities();

	/**
	 * 
	 * @param Location
	 * @return 
	 */
	FVector2D WorldToCanvas(const FVector& Location) const;

protected:
	UPROPERTY(Config, meta=(ForceUnits="cm"))
	float CanvasPixelWorldSize;

	UPROPERTY(Config)
	TSoftObjectPtr<UTextureRenderTarget2D> SoftRenderTarget;

	UPROPERTY(Config)
	FName WorldPlaneName;

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
