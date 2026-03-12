// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Enemy/BaseEnemy/BaseEnemyCharacter.h"
#include "BaseSpitterEnemy.generated.h"

class USpitterDataAsset;

UCLASS()
class BULLETANT_API ABaseSpitterEnemy : public ABaseEnemyCharacter
{
	GENERATED_BODY()

public:
	void StartSpit();
	void CheckContinousSpit();
	void StopSpit();
	
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Spitter")
	TObjectPtr<USpitterDataAsset> SpitterDataAsset;

	FTimerHandle DamageChecker;
};
