// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/DataTable.h"
#include "Enemy/BaseEnemy/BaseEnemyCharacter.h"
#include "Enemy/Tribe/TribeEntry.h"
#include "EnemySpawnerEntry.generated.h"

class ABaseEnemyCharacter;

USTRUCT(BlueprintType)
struct BULLETANT_API FSpawnEnemyData
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<ABaseEnemyCharacter> EnemyClass;

	UPROPERTY(EditDefaultsOnly, meta = (RowType = "TribeEntry"))
	FDataTableRowHandle TribeType;

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

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FSpawnEnemyData> SpawnEnemyDataArray;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 SpawnMinDistance = 2000;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 SpawnMaxDistance = 3000;
};