// Copyright (c) 2025 Xist.GG

#pragma once

#include "MassEntityElementTypes.h"

#include "XmsFragments.generated.h"

/**
 * FXmsF_Transform
 */
USTRUCT()
struct FXmsF_Transform
	: public FMassFragment
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	FVector Location = FVector::ZeroVector;

	UPROPERTY(EditAnywhere)
	FRotator Rotation = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere)
	FVector Scale3D = FVector::OneVector;
};
