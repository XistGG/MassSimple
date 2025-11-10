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

	UPROPERTY(VisibleAnywhere)
	FVector Location = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere)
	FRotator Rotation = FRotator::ZeroRotator;

	UPROPERTY(VisibleAnywhere)
	FVector Scale3D = FVector::OneVector;
};
