// Copyright (c) 2025 Xist.GG LLC

#include "XmsRepSubsystem.h"

#include "Engine/World.h"

//----------------------------------------------------------------------//
//  UXmsRepSubsystem
//----------------------------------------------------------------------//

UXmsRepSubsystem* UXmsRepSubsystem::Get(TNotNull<const UWorld*> World)
{
	UXmsRepSubsystem* Result = World->GetSubsystem<UXmsRepSubsystem>();
	return Result;
}

UXmsRepSubsystem& UXmsRepSubsystem::GetChecked(TNotNull<const UWorld*> World)
{
	UXmsRepSubsystem* Result = Get(World);
	check(Result);
	return *Result;
}

UXmsRepSubsystem::UXmsRepSubsystem()
{
	DataSerialNumber = 0;
	EntityDataCurrentPage = 0;
	EntityDataTempPage = 1;
}

void UXmsRepSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UXmsRepSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

void UXmsRepSubsystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateEntities();
}

TStatId UXmsRepSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UXmsRepSubsystem, STATGROUP_Tickables);
}

void UXmsRepSubsystem::MassPrepare(const int32 ExpectedNum)
{
	EntityDataPages[EntityDataTempPage].Reset(ExpectedNum);
}

void UXmsRepSubsystem::MassAppend(const TArray<const FXmsEntityRepresentationData>& Entities)
{
	EntityDataPages[EntityDataTempPage].Append(Entities);
}

void UXmsRepSubsystem::MassCommit()
{
	QUICK_SCOPE_CYCLE_COUNTER(UXmsRepSubsystem_Commit);

	{
		UE::TWriteScopeLock WriteLock (EntityDataPageLock);

		DataSerialNumber += 1;

		EntityDataCurrentPage = EntityDataTempPage;
		if (++EntityDataTempPage >= EntityDataPages.Num())
		{
			EntityDataTempPage = 0;
		}

		EntityDataCurrentPage = not EntityDataCurrentPage;
		EntityDataTempPage = not EntityDataCurrentPage;
	}
}

void UXmsRepSubsystem::UpdateEntities()
{
	QUICK_SCOPE_CYCLE_COUNTER(UXmsRepSubsystem_UpdateEntities);

	int32 CurrentPage;
	{
		UE::TReadScopeLock ReadLock (EntityDataPageLock);
		CurrentPage = EntityDataCurrentPage;  // COPY the data
	}

	TArray<const FXmsEntityRepresentationData>* CurrentData = &EntityDataPages[CurrentPage];

	// TODO
}
