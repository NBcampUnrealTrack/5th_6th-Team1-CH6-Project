// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Enemy/BaseEnemy/BaseEnemyCharacter.h"
#include "EggCharacter.generated.h"

/**
 * 
 */
UCLASS()
class BULLETANT_API AEggCharacter : public ABaseEnemyCharacter
{
	GENERATED_BODY()

public:
	virtual bool ShouldCallAfterAttack() override;
	
	virtual void AfterAttack() override;
};
