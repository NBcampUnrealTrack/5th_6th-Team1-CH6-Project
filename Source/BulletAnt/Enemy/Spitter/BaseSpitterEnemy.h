// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Enemy/BaseEnemy/BaseEnemyCharacter.h"
#include "BaseSpitterEnemy.generated.h"

/**
 * 
 */
UCLASS()
class BULLETANT_API ABaseSpitterEnemy : public ABaseEnemyCharacter
{
	GENERATED_BODY()

public:
	void StartSpit();
	void StopSpit();
	
};
