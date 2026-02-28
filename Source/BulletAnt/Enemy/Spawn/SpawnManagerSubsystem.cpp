// Fill out your copyright notice in the Description page of Project Settings.

#include "Enemy/Spawn/SpawnManagerSubsystem.h"
#include "BulletAnt/Common/BAWorldSettings.h"
#include "Enemy/Spawn/EnemySpawnerEntry.h"
#include "Enemy/BaseEnemy/BaseEnemyCharacter.h"
#include "Building/BaseCore.h"
#include "Framework/BAGameState.h"

void USpawnManagerSubsystem::OnEnemyDie()
{
	//AliveEnemyCount--;
	//if (AliveEnemyCount == 0 && WaveIndex <= MaxWaveIndex)
	//{
	//	WaveIndex++;
	//	GetWorld()->GetTimerManager().SetTimer(WaveTimer, this, &USpawnManagerSubsystem::StartWave, 1.f, false);
	//}
}

int USpawnManagerSubsystem::GetWavePreparationTime() const
{
	return WavePreparationTime;
}

void USpawnManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	if (!ensureMsgf(IsValid(GetWorld()), TEXT("SpawnManagerSubsystem : GetWorld Is NULL")))
	{
		return;
	}

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
		return false;
	}

	UWorld* World = Cast<UWorld>(Outer);
	if (IsValid((World)))
	{
		if (World->GetNetMode() == ENetMode::NM_Client)
		{
			return false;
		}
		
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

	WavePreparationTime = Row->WavePreparationTime;

	GetWorld()->GetTimerManager().SetTimer(WaveTimer, this, &USpawnManagerSubsystem::UpdatePreparationTime, 1.f, true);
}

void USpawnManagerSubsystem::UpdatePreparationTime()
{
	WavePreparationTime--;

	if (WavePreparationTime <= 0)
	{
		StartWave();
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

		WaveIndex++;
		if (CanStartWave())
		{
			PrepareWave();
		}
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
	
	GetWorld()->GetTimerManager().SetTimer(
		SpawnTimer,
		this, 
		&USpawnManagerSubsystem::SpawnEnemies, 
		Row->SpawnEnemyDataArray[SpawnEnemyDataIdx].SpawnInterval, 
		false
	);
	SpawnEnemyDataIdx++;
}