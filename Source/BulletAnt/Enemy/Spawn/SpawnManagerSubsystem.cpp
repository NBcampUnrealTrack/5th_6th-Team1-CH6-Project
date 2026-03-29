// Fill out your copyright notice in the Description page of Project Settings.

#include "Enemy/Spawn/SpawnManagerSubsystem.h"
#include "BulletAnt/Common/BAWorldSettings.h"
#include "Enemy/Spawn/EnemySpawnerEntry.h"
#include "Enemy/BaseEnemy/BaseEnemyCharacter.h"
#include "Building/BaseCore.h"
#include "Framework/BAGameState.h"
#include "Enemy/Spawn/SpawnLocationManager.h"

void USpawnManagerSubsystem::OnEnemyDie()
{
	//AliveEnemyCount--;
	//if (AliveEnemyCount == 0 && WaveIndex <= MaxWaveIndex)
	//{
	//	WaveIndex++;
	//	GetWorld()->GetTimerManager().SetTimer(WaveTimer, this, &USpawnManagerSubsystem::StartWave, 1.f, false);
	//}
}

void USpawnManagerSubsystem::SetCachedSpawnLocationManager(ASpawnLocationManager* InSpawnLocationManager)
{
	CachedSpawnLocationManager = InSpawnLocationManager;
}

void USpawnManagerSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	UWorld* World = GetWorld();
	if (!ensureMsgf(IsValid(World), TEXT("SpawnManagerSubsystem : GetWorld Is NULL")))
	{
		return;
	}
	if (World->GetNetMode() == ENetMode::NM_Client)
	{
		return;
	}
	CachedGameState = World->GetGameState<ABAGameState>();

	if (IsValid(EnemySpawnHandle.DataTable) == false)
	{
		SetSpawnDataTable();
	}
	if (!ensureMsgf(IsValid(EnemySpawnHandle.DataTable), TEXT("SpawnManagerSubsystem Initialize : DataTable Error")))
	{
		return;
	}
	MaxWaveIndex = EnemySpawnHandle.DataTable->GetRowMap().Num();

	if (CanStartWave())
	{
		PrepareWave();
	}
}

void USpawnManagerSubsystem::OnWorldEndPlay(UWorld& InWorld)
{
	Super::OnWorldEndPlay(InWorld);

	if (IsValid(GetWorld()))
	{
		GetWorld()->GetTimerManager().ClearAllTimersForObject(this);
	}
}

bool USpawnManagerSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	if (!Super::ShouldCreateSubsystem(Outer))
	{
		return false;
	}

	UWorld* World = Cast<UWorld>(Outer);
	if (IsValid((World)))
	{		
		if (!(World->IsGameWorld() || World->IsPlayInEditor()))
		{
			return false;
		}

		FString LevelName = World->GetMapName();
		if (LevelName.Contains(TEXT("TestMap")) || LevelName.Contains(TEXT("MainLevel")))
		{
			return true; 
		}
	}
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

bool USpawnManagerSubsystem::CanStartWave()
{
	if (WaveIndex == 0)
	{
		return true;
	}

	if (!IsValid(CachedGameState->GetTargetCore()))
	{
		return false;
	}

	if (WaveIndex >= MaxWaveIndex)
	{
		// Game Win Logic (적 모두 처치 시 게임엔딩)
		return false;
	}
	return true;
}

void USpawnManagerSubsystem::PrepareWave()
{
	const FName RowName = FName(*FString::Printf(TEXT("Wave%d"), WaveIndex + 1));
	EnemySpawnHandle.RowName = RowName;

	FEnemySpawnerEntry* Row = EnemySpawnHandle.GetRow<FEnemySpawnerEntry>(SpawnContextString);
	if (Row == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("SpawnManagerSubsystem-SpawnEnemies : GetRow Error"));
		return;
	}

	bIsWaveStarted = false;

	int32 SpawnTime = Row->SpawnTime;
	int32 WavePreparationTime = Row->WavePreparationTime;
	int32 InitWavePreparationTime = SpawnTime + WavePreparationTime;
	CachedGameState->SetSpawnTime(SpawnTime);
	CachedGameState->SetWavePreparationTime(InitWavePreparationTime);
	CachedGameState->SetInitWavePreparationTime(InitWavePreparationTime);
	CachedGameState->OnRep_WavePreparationTime();
	CachedGameState->SetDate(WaveIndex + 1);
	OnInitWaveTimeChanged.Broadcast(InitWavePreparationTime);

	GetWorld()->GetTimerManager().SetTimer(WaveTimer, this, &USpawnManagerSubsystem::UpdatePreparationTime, 1.f, true);
}

void USpawnManagerSubsystem::UpdatePreparationTime()
{
	int32 WavePreparationTime = CachedGameState->GetWavePreparationTime();
	WavePreparationTime--;
	CachedGameState->SetWavePreparationTime(WavePreparationTime);
	CachedGameState->OnRep_WavePreparationTime();

	if (!bIsWaveStarted && WavePreparationTime <= CachedGameState->GetSpawnTime())
	{
		bIsWaveStarted = true;
		StartWave();
	}
	if (WavePreparationTime == 0)
	{
		UWorld* World = GetWorld();
		if (!IsValid(World))
		{
			return;
		}
		World->GetTimerManager().ClearTimer(WaveTimer);

		ChangeWave();
	}
}

void USpawnManagerSubsystem::StartWave()
{
	if (IsValid(CachedGameState))
	{
		TargetCore = CachedGameState->GetTargetCore();
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
	if (!IsValid(CachedGameState->GetTargetCore()))
	{
		return;
	}
	if (!IsValid(CachedSpawnLocationManager))
	{
		return;
	}
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
		return;
	}

	TSubclassOf<ABaseEnemyCharacter> EnemyClass = Row->SpawnEnemyDataArray[SpawnEnemyDataIdx].EnemyClass;
	if (EnemyClass == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("SpawnManagerSubsystem-SpawnEnemies : EnemyClass Error"));
		return;
	}

	const int32 Count = Row->SpawnEnemyDataArray[SpawnEnemyDataIdx].Count;
	UTribeDataAsset* TribeDataAsset = Row->SpawnEnemyDataArray[SpawnEnemyDataIdx].TribeType;
	ensureMsgf(TribeDataAsset, TEXT("SpawnTable Tribe Missing"));

	for (int32 j = 0; j < Count; j++)
	{
		FVector SpawnLocation = CachedSpawnLocationManager->GetRandomSpawnLocation();
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
			if (IsValid(TribeDataAsset))
			{
				Enemy->SetTribeType(TribeDataAsset);
				Enemy->OnRep_TribeType();
			}
		}
		else
		{
			j--;
		}
	}

	int NextSpawnTime = Row->SpawnEnemyDataArray[SpawnEnemyDataIdx].SpawnInterval;
	SpawnEnemyDataIdx++;
	if (NextSpawnTime == 0)
	{
		SpawnEnemies();
	}
	else
	{
		GetWorld()->GetTimerManager().SetTimer(
			SpawnTimer,
			this,
			&USpawnManagerSubsystem::SpawnEnemies,
			NextSpawnTime,
			false
		);
	}
}

void USpawnManagerSubsystem::ChangeWave()
{
	SpawnEnemyDataIdx = 0;

	WaveIndex++;
	if (CanStartWave())
	{
		PrepareWave();
	}
}
