// Copyright (c) 2025 Xist.GG LLC

#pragma once

#include "XmsEntityRegistryListener.h"
#include "XmsEntityRegistryListener_Wisp.generated.h"

class UNiagaraSystem;

/**
 * AXmsEntityRegistryListener_Wisp
 */
UCLASS()
class XMS_API AXmsEntityRegistryListener_Wisp
	: public AXmsEntityRegistryListener
{
	GENERATED_BODY()

public:
	// Set Class Defaults
	AXmsEntityRegistryListener_Wisp(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~Begin UObject interface
	virtual void BeginPlay() override;
	//~End UObject interface

	//~Begin AXmsEntityRegistryListener interface
	virtual void NativeOnObservedEntitiesCreated(const EXmsEntityMetaType& MetaType, const TArray<UXmsRegistrySubsystem::FEntityContext>& EntityContexts) override;
	virtual void NativeOnObservedEntitiesDestroyed(const EXmsEntityMetaType& MetaType, const TArray<UXmsRegistrySubsystem::FEntityContext>& EntityContexts) override;
	//~End AXmsEntityRegistryListener interface

protected:
	UPROPERTY(Config, EditAnywhere, Category=Xms)
	FString NiagaraSystemPath;

	UPROPERTY(Transient)
	TObjectPtr<UNiagaraSystem> NiagaraSystem;
};
