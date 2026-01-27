// Fill out your copyright notice in the Description page of Project Settings.

#include "Enemy/Spawn/SpawnManagerSubsystem.h"
#include "BulletAnt/Common/BAWorldSettings.h"
#include "Enemy/Spawn/EnemySpawnerEntry.h"

void USpawnManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	WaveIndex = 0;
	AliveEnemyCount = 0;
	
	checkf(IsValid(GetWorld()), TEXT("SpawnManagerSubsystem : GetWorld Is NULL"));
	
	GetWorld()->GetTimerManager().SetTimer(SpawnTimer, this, &USpawnManagerSubsystem::StartWave, 600, false);
	
	// TargetActor = GetWorld()->GetFirstPlayerController()->GetPawn();
	//
	// UE_LOG(LogTemp, Warning, TEXT("Subsystem Initialize, StartTime : %f, TargetActor Name : %s"), WorldStartTime, *TargetActor->GetName())
}

void USpawnManagerSubsystem::Deinitialize()
{
	Super::Deinitialize();
	
	SetSpawnDataTable();	
	GetWorld()->GetTimerManager().ClearAllTimersForObject(this);
}

void USpawnManagerSubsystem::SetSpawnDataTable()
{
	AWorldSettings* WorldSettings = GetWorld()->GetWorldSettings();
	checkf(IsValid(WorldSettings), TEXT("SpawnManagerSubsystem : GetWorldSettings Error"));
	ABAWorldSettings* BAWorldSettings = Cast<ABAWorldSettings>(WorldSettings);
	checkf(IsValid(BAWorldSettings), TEXT("SpawnManagerSubsystem : Cast BAWorldSettings Error"));
	SpawnDataTable = BAWorldSettings->SpawnTable;
}

void USpawnManagerSubsystem::StartWave()
{
	GetWorld()->GetTimerManager().ClearTimer(SpawnTimer);
	SpawnEnemies(WaveIndex);
}

void USpawnManagerSubsystem::SpawnEnemies(int32 InWaveIndex)
{
	if (IsValid(SpawnDataTable) == false)
	{
		return;
	}

	static const FString ContextString(TEXT("EnemySpawnContext"));
	FEnemySpawnerEntry* Row = SpawnDataTable->FindRow<FEnemySpawnerEntry>(FName(TEXT("Wave1")), ContextString);
	
	for (int i = 0; i < Row->SpawnEnemyDataArray.Num(); i++)
	{
		for (int j = 0; j < Row->SpawnEnemyDataArray[i].Count(); j++)
		{
			// spawn
		}
	}
}
