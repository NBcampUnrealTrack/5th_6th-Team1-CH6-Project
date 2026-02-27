// Fill out your copyright notice in the Description page of Project Settings.

#include "Enemy/Spawn/SpawnManagerSubsystem.h"
#include "BulletAnt/Common/BAWorldSettings.h"
#include "Enemy/Spawn/EnemySpawnerEntry.h"
#include "Enemy/BaseEnemy/BaseEnemyCharacter.h"
#include "Building/BaseCore.h"
#include "Framework/BAGameState.h"

void USpawnManagerSubsystem::OnEnemyDie()
{
	AliveEnemyCount--;
	if (AliveEnemyCount == 0 && WaveIndex <= MaxWaveIndex)
	{
		WaveIndex++;
		GetWorld()->GetTimerManager().SetTimer(WaveTimer, this, &USpawnManagerSubsystem::StartWave, 1.f, false);
	}
}

void USpawnManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	if (!ensureMsgf(IsValid(GetWorld()), TEXT("SpawnManagerSubsystem : GetWorld Is NULL")))
	{
		return;
	}

	GetWorld()->GetTimerManager().SetTimer(WaveTimer, this, &USpawnManagerSubsystem::StartWave, 180.f, false);
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
		if (LevelName.Contains(TEXT("TestMap")) || LevelName.Contains(TEXT("MainLevel")))
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
	if (!ensureMsgf(IsValid(WorldSettings), TEXT("SpawnManagerSubsystem : GetWorldSettings Error")))
	{
		return;
	}
	ABAWorldSettings* BAWorldSettings = Cast<ABAWorldSettings>(WorldSettings);
	if (!ensureMsgf(IsValid(BAWorldSettings), TEXT("SpawnManagerSubsystem : Cast BAWorldSettings Error")))
	{
		return;
	}
	EnemySpawnHandle.DataTable = BAWorldSettings->SpawnTable;
	if (!ensureMsgf(IsValid(EnemySpawnHandle.DataTable), TEXT("SpawnManagerSubsystem : SetSpawnDataTable Error")))
	{
		return;
	}
}

void USpawnManagerSubsystem::StartWave()
{
	UWorld* World = GetWorld();
	if (!IsValid(World))
	{
		return;
	}
	World->GetTimerManager().ClearTimer(WaveTimer);

	if (ABAGameState* GS = World->GetGameState<ABAGameState>())
	{
		TargetCore = GS->GetTargetCore();
	}
	
	if (IsValid(EnemySpawnHandle.DataTable) == false)
	{
		SetSpawnDataTable();
	}
	
	const FName RowName = FName(*FString::Printf(TEXT("Wave%d"), WaveIndex + 1));
	EnemySpawnHandle.RowName = RowName;
	
	SpawnEnemies();
}

void USpawnManagerSubsystem::SpawnEnemies()
{	
	if (IsValid(EnemySpawnHandle.DataTable) == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("SpawnManagerSubsystem : DataTable Error"));
		return;
	}
	
	FEnemySpawnerEntry* Row = EnemySpawnHandle.GetRow<FEnemySpawnerEntry>(SpawnContextString); 
	if (Row == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("SpawnManagerSubsystem-SpawnEnemies : GetRow Error"));
		return;
	}
	
	if (SpawnEnemyDataIdx >= Row->SpawnEnemyDataArray.Num())
	{
		SpawnEnemyDataIdx = 0;
		return;
	}
	
	TSubclassOf<ABaseEnemyCharacter> EnemyClass = Row->SpawnEnemyDataArray[SpawnEnemyDataIdx].EnemyClass;
	if (EnemyClass == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("SpawnManagerSubsystem-SpawnEnemies : EnemyClass Error"));
		return;
	}
	
	const int32 Count = Row->SpawnEnemyDataArray[SpawnEnemyDataIdx].Count;
	const int32 MinDistance = Row->SpawnMinDistance;
	const int32 MaxDistance = Row->SpawnMaxDistance;
	UTribeDataAsset* TribeDataAsset = Row->SpawnEnemyDataArray[SpawnEnemyDataIdx].TribeType;
	ensureMsgf(TribeDataAsset, TEXT("SpawnTable Tribe Missing"));
		
	for (int32 j = 0; j < Count; j++)
	{
		FVector RandomDirection = FMath::VRand();
		RandomDirection.Z = 0.0f; 
		RandomDirection.Normalize();
		float RandomDistance = FMath::FRandRange(static_cast<float>(MinDistance), static_cast<float>(MaxDistance));
		
		FVector SpawnLocation = TargetCore->GetActorLocation() + (RandomDirection * RandomDistance);
		SpawnLocation.Z += 50.f;

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
		ABaseEnemyCharacter* Enemy = GetWorld()->SpawnActor<ABaseEnemyCharacter>(
			EnemyClass,
			SpawnLocation,
			FRotator::ZeroRotator,
			SpawnParams
		);
		if (IsValid(Enemy))
		{
			AliveEnemyCount++;
			Enemy->SetTribeType(TribeDataAsset);
			Enemy->ApplyTribe();
		}
	}
	
	SpawnEnemyDataIdx++;
	if (SpawnEnemyDataIdx == Row->SpawnEnemyDataArray.Num())
	{
		SpawnEnemyDataIdx = 0;
		return;
	}
	
	GetWorld()->GetTimerManager().SetTimer(
		SpawnTimer,
		this, 
		&USpawnManagerSubsystem::SpawnEnemies, 
		Row->SpawnEnemyDataArray[SpawnEnemyDataIdx].SpawnInterval, 
		false);
}