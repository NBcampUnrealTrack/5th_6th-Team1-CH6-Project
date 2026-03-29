// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "LevelSpanwerDataAsset.generated.h"

class ABaseEnemyCharacter;
class UTribeDataAsset;

UCLASS()
class BULLETANT_API ULevelSpanwerDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly)
	TArray<TSubclassOf<ABaseEnemyCharacter>> EnemyClass;

	UPROPERTY(EditDefaultsOnly)
	TArray<TObjectPtr<UTribeDataAsset>> TribeType;

	UPROPERTY(EditDefaultsOnly)
	int32 AreaForPerSpawn = 200000;
};
