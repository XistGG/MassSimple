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
	Rock = 1 UMETA(DisplayName = "Rock"),
	Tree = 2 UMETA(DisplayName = "Tree"),
	Wisp = 3 UMETA(DisplayName = "Wisp"),
	// Add new MetaTypes here, then update MAX accordingly
	MAX = 4 UMETA(Hidden = true),  // NOTICE: Always set MAX to the exclusive upper limit of valid values
};

/**
 * FXmsCSF_MetaData
 */
USTRUCT()
struct FXmsCSF_MetaData
	: public FMassConstSharedFragment
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	EXmsEntityMetaType MetaType = EXmsEntityMetaType::None;

	// MetaSubType
	// MetaThis
	// MetaThat

	inline bool IsValid() const;
};

inline bool FXmsCSF_MetaData::IsValid() const
{
	return MetaType != EXmsEntityMetaType::None
		&& static_cast<uint8>(MetaType) < static_cast<uint8>(EXmsEntityMetaType::MAX);
}
