// Copyright (c) 2025 Xist.GG

#pragma once

#include "MassObserverProcessor.h"

#include "XmsEntityRegistryProcessors.generated.h"

/**
 * UXmsEntityCreated
 */
UCLASS()
class XMS_API UXmsEntityCreated
	: public UMassObserverProcessor
{
	GENERATED_BODY()

	UXmsEntityCreated();

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

	FMassEntityQuery Query;
};

/**
 * UXmsEntityDestroyed
 */
UCLASS()
class XMS_API UXmsEntityDestroyed
	: public UMassObserverProcessor
{
	GENERATED_BODY()

	UXmsEntityDestroyed();

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

	FMassEntityQuery Query;
};
