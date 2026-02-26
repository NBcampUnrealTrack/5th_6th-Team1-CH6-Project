// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Net/UnrealNetwork.h"
#include "GameplayTagContainer.h"
#include "AbilitySystemComponent.h" 
#include "AbilitySystemGlobals.h"
#include "Player/BACharacter.h"
#include "BAAnimInstance.generated.h"

/**
 * 
 */

class UCharacterMovementComponent;

UCLASS()
class BULLETANT_API UBAAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
public:
	// 초기화 함수
	virtual void NativeInitializeAnimation() override;

	// 매 프레임 실행 함수
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	void SetIsFiring(bool InFiring);


protected:
	FRotator CameraTargetOffset();

public:
	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	EEquipmentType CurrentEquipmentType;

protected:
	// 매번 Cast 안하기 위한 포인터
	UPROPERTY(BlueprintReadOnly, Category = "AnimCharacter")
	TObjectPtr<ABACharacter> Character;

	// 매번 Cast 안하기 위한 포인터
	UPROPERTY(BlueprintReadOnly, Category = "AnimCharacter")
	TObjectPtr <UCharacterMovementComponent> Movement;

	FGameplayTag Tag_Ranged;
	FGameplayTag Tag_Mining;
    FGameplayTag Tag_Melee;

	UPROPERTY(BlueprintReadOnly, Category = "AnimCharacter")
	float GroundSpeed;

	float TargetPitch;
	float TargetYaw;
	
	UPROPERTY(BlueprintReadOnly, Category = "AnimCharacter")
	float AOPitch;
	UPROPERTY(BlueprintReadOnly, Category = "AnimCharacter")
	float AOYaw;

	UPROPERTY(BlueprintReadOnly, Category = "AnimCharacter")
	float Direction;
	// 조준 상태
	UPROPERTY(BlueprintReadOnly, Category = "AnimCharacter")
	bool bIsAiming;

	UPROPERTY(BlueprintReadOnly, Category = "AnimCharacter")
	bool bIsFiring = false;

	// 공중 상태
	UPROPERTY(BlueprintReadOnly, Category = "AnimCharacter")
	bool bIsFalling;

	// 달리기 상태
	UPROPERTY(BlueprintReadOnly, Category = "AnimCharacter")
	bool bIsRunning;

	// 앉기 상태
	UPROPERTY(BlueprintReadOnly, Category = "AnimCharacter")
	bool bIsCrouch;
	UPROPERTY(BlueprintReadOnly, Category = "AnimCharacter")
	bool bIsTurning;

	//수직 이동 속도
	UPROPERTY(BlueprintReadOnly, Category = "AnimCharacter")
	float VerticalVelocity;

	FRotator PreviousRotation;

	UPROPERTY(BlueprintReadOnly, Category = "TurnInPlace")
	float RootYawOffset;
};
