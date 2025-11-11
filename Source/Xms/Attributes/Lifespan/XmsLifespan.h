// Copyright (c) 2025 Xist.GG

#pragma once

#include "MassEntityElementTypes.h"

#include "XmsLifespan.generated.h"

/**
 * EXmsEntityLifespanFlags
 */
UENUM(BlueprintType, meta=(Bitflags))
enum class EXmsEntityLifespanFlags : uint8
{
	None = 0 UMETA(DisplayName = "None"),
	Immortal = 1 << 0 UMETA(DisplayName = "Immortal"),
};

ENUM_CLASS_FLAGS(EXmsEntityLifespanFlags)

/**
 * FXmsF_Lifespan
 */
USTRUCT()
struct FXmsF_Lifespan
	: public FMassFragment
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere)
	EXmsEntityLifespanFlags Flags = EXmsEntityLifespanFlags::None;

	UPROPERTY(VisibleAnywhere)
	float CurrentAge = 0.;

	UPROPERTY(VisibleAnywhere)
	float MaxAge = 0.;

	inline bool IsImmortal() const;
};

inline bool FXmsF_Lifespan::IsImmortal() const
{
	return (Flags & EXmsEntityLifespanFlags::Immortal) == EXmsEntityLifespanFlags::Immortal;
}
