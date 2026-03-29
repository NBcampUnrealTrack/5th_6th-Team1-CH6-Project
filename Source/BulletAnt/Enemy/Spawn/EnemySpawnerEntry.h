// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/DataTable.h"
#include "Enemy/BaseEnemy/BaseEnemyCharacter.h"
#include "Enemy/DataAsset/TribeDataAsset.h"
#include "EnemySpawnerEntry.generated.h"

class ABaseEnemyCharacter;

USTRUCT(BlueprintType)
struct BULLETANT_API FSpawnEnemyData
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<ABaseEnemyCharacter> EnemyClass;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UTribeDataAsset> TribeType;

	UPROPERTY(EditDefaultsOnly)
	int32 Count = 10;

	UPROPERTY(EditDefaultsOnly)
	float SpawnInterval = 1.f;
};

USTRUCT(BlueprintType)
struct BULLETANT_API FEnemySpawnerEntry : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 WaveIndex = 0;

	UPROPERTY(EditDefaultsOnly)
	int32 WavePreparationTime = 300;

	UPROPERTY(EditDefaultsOnly)
	int32 SpawnTime = 100;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FSpawnEnemyData> SpawnEnemyDataArray;
};