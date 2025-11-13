// Copyright (c) 2025 Xist.GG

#pragma once

#include "MassProcessor.h"

#include "XmsLifespanEnforcer.generated.h"

/**
 * UXmsLifespanEnforcer
 */
UCLASS()
class XMS_API UXmsLifespanEnforcer
	: public UMassProcessor
{
	GENERATED_BODY()

	UXmsLifespanEnforcer();

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

	FMassEntityQuery Query;
};
