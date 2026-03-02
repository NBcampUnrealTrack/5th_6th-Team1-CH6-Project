// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "SpawnManagerSubsystem.generated.h"

class ABaseCore;
class ABAGameState;

UCLASS()
class BULLETANT_API USpawnManagerSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	void OnEnemyDie();

protected:
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	virtual void OnWorldEndPlay(UWorld& InWorld) override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	void SetSpawnDataTable();
	bool CanStartWave();
	void PrepareWave();
	void UpdatePreparationTime();
	void StartWave();
	void SpawnEnemies();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<ABaseCore> TargetCore;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
	FDataTableRowHandle EnemySpawnHandle;

	inline static const FString SpawnContextString = (TEXT("EnemySpawnContext"));

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spawning")
	int32 WaveIndex = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spawning")
	int32 MaxWaveIndex;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spawning")
	int32 AliveEnemyCount = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spawning")
	int32 SpawnEnemyDataIdx = 0;

	UPROPERTY()
	TObjectPtr<ABAGameState> CachedGameState;

	FTimerHandle WaveTimer;
	FTimerHandle SpawnTimer;
};
