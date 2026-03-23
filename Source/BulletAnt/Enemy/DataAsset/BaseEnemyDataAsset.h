// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayEffectTypes.h"	
#include "GameplayEffect.h"
#include "Weapon/Data/MeleeWeaponDataAsset.h"
#include "NiagaraFunctionLibrary.h"
#include "BaseEnemyDataAsset.generated.h"

struct FGameplayTagContainer;

USTRUCT()
struct FAttackDataAsset
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly)
	int32 Distance = 0;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UWeaponDataAsset> AttackDataAsset;
};

UCLASS()
class BULLETANT_API UBaseEnemyDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:

	UPROPERTY(EditDefaultsOnly, Category = "GAS|Abilities")
	TArray<TSubclassOf<UGameplayAbility>> DefaultAbilities;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|Effects")
	TArray<TSubclassOf<UGameplayEffect>> DefaultEffects;

#pragma region BaseStat

	UPROPERTY(EditDefaultsOnly, Category = "BaseStat")
	int32 Health = 100.f;

#pragma endregion

#pragma region Move
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Move")
	float MoveSpeed = 600.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Move")
	TSubclassOf<UGameplayEffect> MoveEffect;

	UPROPERTY(EditDefaultsOnly, Category = "Move")
	FGameplayTag MoveStateTag;

	UPROPERTY(EditDefaultsOnly, Category = "Move")
	float AcceptanceRadius = 100.f;


#pragma endregion

#pragma region Attack

	UPROPERTY(EditDefaultsOnly, Category = "Attack")
	FGameplayTag AttackStateTag;

	UPROPERTY(EditDefaultsOnly, Category = "Attack")
	TArray<FAttackDataAsset> BaseEnemyAttackDataAssetArray;

#pragma endregion

#pragma region Rotate

	UPROPERTY(EditDefaultsOnly, Category = "Rotate")
	float RotationRate = 360.f;

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
	float SenseAngle = 160.f;

#pragma endregion

#pragma region Intrude

	UPROPERTY(EditDefaultsOnly, Category = "Intrude")
	float IntrudeTime = 2.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Intrude")
	TSubclassOf<UGameplayEffect> IntrudeEffect;

#pragma endregion

#pragma region Death

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Death")
	TSubclassOf<UGameplayEffect> DeathEffect;

	UPROPERTY(EditDefaultsOnly, Category = "Death")
	FGameplayTag DeathStateTag;

	UPROPERTY(EditDefaultsOnly, Category = "Death")
	TObjectPtr<UAnimMontage> DieAnimMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Death")
	float DeathTime = 5.f;

#pragma endregion

#pragma region Spawn

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spawn")
	TObjectPtr<UNiagaraSystem> SpawnEffect;

#pragma endregion
};
