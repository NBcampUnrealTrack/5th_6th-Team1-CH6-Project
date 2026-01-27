// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "BAAnimInstance.generated.h"

class ABACharacter;
class UCharacterMovementComponent;

UCLASS()
class BULLETANT_API UBAAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	// ??? ??
	virtual void NativeInitializeAnimation() override;

	// ? ??? ?? ??
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

protected:
	// ?? Cast ??? ?? ???
	UPROPERTY(BlueprintReadOnly, Category = "Character")
	ABACharacter* Character;

	UPROPERTY(BlueprintReadOnly, Category = "Character")
	UCharacterMovementComponent* Movement;

	UPROPERTY(BlueprintReadOnly, Category = "Character")
	float GroundSpeed;

	UPROPERTY(BlueprintReadOnly, Category = "Character")
	float Direction;

	// ?? ??
	UPROPERTY(BlueprintReadOnly, Category = "Character")
	bool bIsAiming;

	// ?? ??
	UPROPERTY(BlueprintReadOnly, Category = "Character")
	bool bIsFalling;

	// ??? ??
	UPROPERTY(BlueprintReadOnly, Category = "Character")
	bool bIsRunning;

	// ?? ??
	UPROPERTY(BlueprintReadOnly, Category = "Character")
	bool bIsCrouched;

	// ?? ?? ??
	UPROPERTY(BlueprintReadOnly, Category = "Character")
	float VerticalVelocity;
};
