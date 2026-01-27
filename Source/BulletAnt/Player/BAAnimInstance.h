// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "BAAnimInstance.generated.h"

/**
 * 
 */

class ABACharacter;
class UCharacterMovementComponent;
UCLASS()
class BULLETANT_API UBAAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
public:
	// Initialize animation
	virtual void NativeInitializeAnimation() override;

	// Called every frame
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

protected:
	// Cached pointer to avoid casting every frame
	UPROPERTY(BlueprintReadOnly, Category = "Character")
	class ABACharacter* Character;

	// Cached pointer to movement component
	UPROPERTY(BlueprintReadOnly, Category = "Character")
	class UCharacterMovementComponent* Movement;

	UPROPERTY(BlueprintReadOnly, Category = "Character")
	float GroundSpeed;

	UPROPERTY(BlueprintReadOnly, Category = "Character")
	float Direction;
};
