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
 * FFXms_MetaData
 */
USTRUCT()
struct FFXms_MetaData
	: public FMassFragment
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere)
	EXmsEntityMetaType MetaType = EXmsEntityMetaType::None;

	// MetaSubType
	// MetaThis
	// MetaThat

	inline bool IsValid() const;
};

inline bool FFXms_MetaData::IsValid() const
{
	return MetaType != EXmsEntityMetaType::None
		&& static_cast<uint8>(MetaType) < static_cast<uint8>(EXmsEntityMetaType::MAX);
}
