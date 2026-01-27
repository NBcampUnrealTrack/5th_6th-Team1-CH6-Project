// Fill out your copyright notice in the Description page of Project Settings.

#include "Enemy/Spawn/SpawnManagerSubsystem.h"
#include "BulletAnt/Common/BAWorldSettings.h"
#include "Enemy/Spawn/EnemySpawnerEntry.h"
#include "Enemy/BaseEnemyCharacter.h"

AActor* USpawnManagerSubsystem::GetTargetActor() const
{
	return TargetActor;
}

void USpawnManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	WaveIndex = 0;
	AliveEnemyCount = 0;
	
	checkf(IsValid(GetWorld()), TEXT("SpawnManagerSubsystem : GetWorld Is NULL"));
	GetWorld()->GetTimerManager().SetTimer(SpawnTimer, this, &USpawnManagerSubsystem::StartWave, 5, false);
	
	// TargetActor = GetWorld()->GetFirstPlayerController()->GetPawn();
	//
	// UE_LOG(LogTemp, Warning, TEXT("Subsystem Initialize, StartTime : %f, TargetActor Name : %s"), WorldStartTime, *TargetActor->GetName())
}

void USpawnManagerSubsystem::Deinitialize()
{
	Super::Deinitialize();
	
	if (IsValid(GetWorld()))
	{
		GetWorld()->GetTimerManager().ClearAllTimersForObject(this);
	}
}

void USpawnManagerSubsystem::SetSpawnDataTable()
{
	AWorldSettings* WorldSettings = GetWorld()->GetWorldSettings();
	checkf(IsValid(WorldSettings), TEXT("SpawnManagerSubsystem : GetWorldSettings Error"));
	ABAWorldSettings* BAWorldSettings = Cast<ABAWorldSettings>(WorldSettings);
	checkf(IsValid(BAWorldSettings), TEXT("SpawnManagerSubsystem : Cast BAWorldSettings Error"));
	SpawnDataTable = BAWorldSettings->SpawnTable;
	checkf(IsValid(SpawnDataTable), TEXT("SpawnManagerSubsystem : SetSpawnDataTable Error"));
}

void USpawnManagerSubsystem::StartWave()
{
	GetWorld()->GetTimerManager().ClearTimer(SpawnTimer);
	
	if (TargetActor == nullptr)
	{
		TargetActor = GetWorld()->GetFirstPlayerController()->GetPawn();
	}
	
	if (IsValid(SpawnDataTable) == false)
	{
		SetSpawnDataTable();
	}
	
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
	
	for (int32 i = 0; i < Row->SpawnEnemyDataArray.Num(); i++)
	{
		TSubclassOf<ABaseEnemyCharacter> EnemyClass = Row->SpawnEnemyDataArray[i].EnemyClass;
		if (EnemyClass == nullptr)
		{
			continue;
		}
		const int32 Count = Row->SpawnEnemyDataArray[i].Count;
		const int32 MinDistance = Row->SpawnMinDistance;
		const int32 MaxDistance = Row->SpawnMaxDistance;
			
		for (int32 j = 0; j < Count; j++)
		{
			int32 SpawnX = FMath::RandRange(MinDistance, MaxDistance) + TargetActor->GetActorLocation().X;
			int32 SpawnY = FMath::RandRange(MinDistance, MaxDistance) + TargetActor->GetActorLocation().Y;
			
			ABaseEnemyCharacter* Enemy = GetWorld()->SpawnActor<ABaseEnemyCharacter>(
				EnemyClass,
				FVector(SpawnX, SpawnY, TargetActor->GetActorLocation().Z),
				FRotator::ZeroRotator
			);
		}
	}
}
