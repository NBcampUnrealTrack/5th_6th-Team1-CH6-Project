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
	// 초기화 함수
	virtual void NativeInitializeAnimation() override;

	// 매 프레임 실행 함수
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

protected:
	// 매번 Cast 안하기 위한 포인터
	UPROPERTY(BlueprintReadOnly, Category = "Character")
	class ABACharacter* Character;

	// 매번 Cast 안하기 위한 포인터
	UPROPERTY(BlueprintReadOnly, Category = "Character")
	class UCharacterMovementComponent* Movement;

	UPROPERTY(BlueprintReadOnly, Category = "Character")
	float GroundSpeed;

	UPROPERTY(BlueprintReadOnly, Category = "Character")
	float Direction;

	// 조준 상태
	UPROPERTY(BlueprintReadOnly, Category = "Character")
	bool bIsAiming;

	// 공중 상태
	UPROPERTY(BlueprintReadOnly, Category = "Character")
	bool bIsFalling;

	// 달리기 상태
	UPROPERTY(BlueprintReadOnly, Category = "Character")
	bool bIsRunning;


	//수직 이동 속도
	UPROPERTY(BlueprintReadOnly, Category = "Character")
	float VerticalVelocity;
};
