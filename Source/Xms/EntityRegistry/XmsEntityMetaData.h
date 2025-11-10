// Copyright (c) 2025 Xist.GG

#pragma once

#include "MassEntityElementTypes.h"

#include "XmsEntityMetaData.generated.h"

/**
 * EXmsEntityMetaType
 */
UENUM()
enum class EXmsEntityMetaType : uint8
{
	None = 0 UMETA(DisplayName = "None"),
	Tree = 1 UMETA(DisplayName = "Tree"),
	MAX = 2 UMETA(Hidden = true),
};

/**
 * FCSFXms_MetaData
 */
USTRUCT()
struct FCSFXms_MetaData
	: public FMassConstSharedFragment
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere)
	EXmsEntityMetaType MetaType = EXmsEntityMetaType::None;

	// MetaSubType
	// MetaThis
	// MetaThat

	inline bool IsValid() const;
};

inline bool FCSFXms_MetaData::IsValid() const
{
	return MetaType != EXmsEntityMetaType::None
		&& static_cast<uint8>(MetaType) < static_cast<uint8>(EXmsEntityMetaType::MAX);
}

USTRUCT()
struct FTXms_Register
	: public FMassTag
{
	GENERATED_BODY()
};
