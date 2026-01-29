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
}

void USpawnManagerSubsystem::Deinitialize()
{
	Super::Deinitialize();
	
	if (IsValid(GetWorld()))
	{
		GetWorld()->GetTimerManager().ClearAllTimersForObject(this);
	}
}

bool USpawnManagerSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	if (!Super::ShouldCreateSubsystem(Outer))
	{
		
		// UE_LOG(LogTemp, Warning, TEXT("1"))
		return false;
	}

	UWorld* World = Cast<UWorld>(Outer);
	if (IsValid((World)))
	{
		if (World->GetNetMode() == ENetMode::NM_Client)
		{
			// UE_LOG(LogTemp, Warning, TEXT("2"))
			return false;
		}
		
		if (!(World->IsGameWorld() || World->IsPlayInEditor()))
		{
			// UE_LOG(LogTemp, Warning, TEXT("3"))
			return false;
		}

		FString LevelName = World->GetMapName();
		if (LevelName.Contains(TEXT("TestMap")))
		{
			// UE_LOG(LogTemp, Warning, TEXT("4"))
			return true; 
		}
	}
	// UE_LOG(LogTemp, Warning, TEXT("5"))
	return false;
}

void USpawnManagerSubsystem::SetSpawnDataTable()
{
	AWorldSettings* WorldSettings = GetWorld()->GetWorldSettings();
	checkf(IsValid(WorldSettings), TEXT("SpawnManagerSubsystem : GetWorldSettings Error"));
	ABAWorldSettings* BAWorldSettings = Cast<ABAWorldSettings>(WorldSettings);
	checkf(IsValid(BAWorldSettings), TEXT("SpawnManagerSubsystem : Cast BAWorldSettings Error"));
	EnemySpawnHandle.DataTable = BAWorldSettings->SpawnTable;
	checkf(IsValid(EnemySpawnHandle.DataTable), TEXT("SpawnManagerSubsystem : SetSpawnDataTable Error"));
}

void USpawnManagerSubsystem::StartWave()
{
	GetWorld()->GetTimerManager().ClearTimer(SpawnTimer);
	
	if (TargetActor == nullptr)
	{
		if (IsValid(GetWorld()->GetFirstPlayerController()))
		{
			TargetActor = GetWorld()->GetFirstPlayerController()->GetPawn();
		}
	}
	
	if (IsValid(EnemySpawnHandle.DataTable) == false)
	{
		SetSpawnDataTable();
	}
	
	SpawnEnemies(WaveIndex);
}

void USpawnManagerSubsystem::SpawnEnemies(int32 InWaveIndex)
{
	UE_LOG(LogTemp, Warning, TEXT("Spawn"))
	if (IsValid(EnemySpawnHandle.DataTable) == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("SpawnManagerSubsystem : DataTable Error"));
		return;
	}
	if (IsValid(TargetActor) == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("SpawnManagerSubsystem : TargetActor Error"));
		return;
	}
	
	static const FString ContextString(TEXT("EnemySpawnContext"));
	const FName RowName = FName(*FString::Printf(TEXT("Wave%d"), InWaveIndex + 1));
	EnemySpawnHandle.RowName = RowName;
	FEnemySpawnerEntry* Row = EnemySpawnHandle.GetRow<FEnemySpawnerEntry>(ContextString); 
	if (Row == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("SpawnManagerSubsystem: Row '%s' not found in DataTable"), *RowName.ToString());
		return;
	}
	
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
			FVector RandomDirection = FMath::VRand();
			RandomDirection.Z = 0.0f; 
			RandomDirection.Normalize();
			float RandomDistance = FMath::FRandRange(static_cast<float>(MinDistance), static_cast<float>(MaxDistance));
			
			FVector SpawnLocation = TargetActor->GetActorLocation() + (RandomDirection * RandomDistance);
			
			ABaseEnemyCharacter* Enemy = GetWorld()->SpawnActor<ABaseEnemyCharacter>(
				EnemyClass,
				SpawnLocation,
				FRotator::ZeroRotator
			);
		}
	}
}
