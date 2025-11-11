// Copyright (c) 2025 Xist.GG

#pragma once

#include "MassProcessor.h"

#include "XmsRepresentationProcessor.generated.h"

/**
 * UXmsRepresentationProcessor
 */
UCLASS()
class XMS_API UXmsRepresentationProcessor
	: public UMassProcessor
{
	GENERATED_BODY()

	UXmsRepresentationProcessor();

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

	FMassEntityQuery Query;
};
