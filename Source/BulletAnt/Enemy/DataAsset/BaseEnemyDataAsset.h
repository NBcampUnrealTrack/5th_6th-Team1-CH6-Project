// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayEffectTypes.h"	
#include "GameplayEffect.h"
#include "Weapon/Data/MeleeWeaponDataAsset.h"
#include "BaseEnemyDataAsset.generated.h"

struct FGameplayTagContainer;

UCLASS()
class BULLETANT_API UBaseEnemyDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:

	UPROPERTY(EditDefaultsOnly, Category = "GAS|Abilities")
	TArray<TSubclassOf<UGameplayAbility>> DefaultAbilities;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|Effects")
	TArray<TSubclassOf<UGameplayEffect>> DefaultEffects;

#pragma region Move

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Move")
	TSubclassOf<UGameplayEffect> MoveEffect;

	UPROPERTY(EditDefaultsOnly, Category = "Move")
	FGameplayTag MoveStateTag;

	UPROPERTY(EditDefaultsOnly, Category = "Move")
	float AcceptanceRadius = 100.f;

	UPROPERTY(EditDefaultsOnly, Category = "Move")
	float RotationRate = 360.f;

#pragma endregion

#pragma region Attack

	UPROPERTY(EditDefaultsOnly, Category = "Attack")
	FGameplayTag AttackStateTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack")
	TObjectPtr<UWeaponDataAsset> BaseEnemyAttackDataAsset;

#pragma endregion

#pragma region Rotate

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rotate")
	TSubclassOf<UGameplayEffect> RotateEffect;

	UPROPERTY(EditDefaultsOnly, Category = "Rotate")
	FGameplayTag RotateStateTag;

	UPROPERTY(EditDefaultsOnly, Category = "Rotate")
	float RotateThreshold = 10.f;	// 임계치 값 이내로 Target과 방향 일치 시, 회전 멈춤

#pragma endregion

#pragma region Perception	

	UPROPERTY(EditDefaultsOnly, Category = "Perception")
	float SenseRadius = 100.f;

	UPROPERTY(EditDefaultsOnly, Category = "Perception")
	float SenseAngle = 120.f;

#pragma endregion

};
