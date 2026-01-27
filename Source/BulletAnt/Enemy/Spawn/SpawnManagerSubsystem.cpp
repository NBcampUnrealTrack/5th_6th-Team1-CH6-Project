// Fill out your copyright notice in the Description page of Project Settings.

#include "Enemy/Spawn/SpawnManagerSubsystem.h"

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
	
	GetWorld()->GetTimerManager().ClearAllTimersForObject(this);
}

void USpawnManagerSubsystem::StartWave()
{
	GetWorld()->GetTimerManager().ClearTimer(SpawnTimer);
	SpawnEnemies(WaveIndex);
}

void USpawnManagerSubsystem::SpawnEnemies(int32 InWaveIndex)
{
}
