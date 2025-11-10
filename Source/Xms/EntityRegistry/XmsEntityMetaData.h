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
	// Add new MetaTypes here, then update MAX accordingly
	MAX = 2 UMETA(Hidden = true),  // NOTICE: Always set MAX to the exclusive upper limit of valid values
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

/**
 * FTXms_Registry
 *
 * An Entity must be tagged with this tag to be seen by the XmsEntityRegistry observer
 * processors, thus any Entity that wants to be in the Registry requires this tag.
 */
USTRUCT()
struct FTXms_Registry
	: public FMassTag
{
	GENERATED_BODY()
};
