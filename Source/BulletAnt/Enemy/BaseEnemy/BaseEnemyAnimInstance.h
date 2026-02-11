// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "BaseEnemyAnimInstance.generated.h"

class ABaseEnemyCharacter;
class UCharacterMovementComponent;

UCLASS()
class BULLETANT_API UBaseEnemyAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

protected:
	UPROPERTY(BlueprintReadOnly, Category = "BaseEnemy")
	TObjectPtr<ABaseEnemyCharacter> Enemy;

	UPROPERTY(BlueprintReadOnly, Category = "BaseEnemy")
	TObjectPtr<UCharacterMovementComponent> MovementComponent;

	UPROPERTY(BlueprintReadOnly, Category = "BaseEnemy")
	FVector Velocity;

	UPROPERTY(BlueprintReadOnly, Category = "BaseEnemy")
	float GroundSpeed;

	UPROPERTY(BlueprintReadOnly, Category = "BaseEnemy")
	float Direction;

	UPROPERTY(BlueprintReadOnly, Category = "BaseEnemy")
	bool bShouldMove;

	UPROPERTY(BlueprintReadOnly, Category = "BaseEnemy")
	bool bIsFalling;
	
};
