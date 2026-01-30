// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayEffectTypes.h"	
#include "GameplayEffect.h"
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
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|Effects")
	TSubclassOf<UGameplayEffect> MoveEffect;
	
	UPROPERTY(EditDefaultsOnly, Category = "GAS|Tag")
	FGameplayTag MoveStateTag;

	// 공격 키를 눌렀을 때의 Tag 설정
	UPROPERTY(EditDefaultsOnly, Category = "GAS|Tag")
	FGameplayTag AttackStateTag;
	
	UPROPERTY(EditDefaultsOnly, Category = "GAS")
	TArray<TSubclassOf<UAttributeSet>> DefaultAttributeSets;	
	
	UPROPERTY(EditDefaultsOnly, Category = "Default")
	float AcceptanceRadius = 100.f;	
};
