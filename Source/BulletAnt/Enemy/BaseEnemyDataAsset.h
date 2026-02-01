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
	
	UPROPERTY(EditDefaultsOnly, Category = "GAS|AttritubeSet")
	TArray<TSubclassOf<UAttributeSet>> DefaultAttributeSets;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Move")
	TSubclassOf<UGameplayEffect> MoveEffect;
	
	UPROPERTY(EditDefaultsOnly, Category = "Move")
	FGameplayTag MoveStateTag;
	
	UPROPERTY(EditDefaultsOnly, Category = "Move")
	float AcceptanceRadius = 100.f;	

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack")
	TSubclassOf<UGameplayEffect> AttackEffect;	

	UPROPERTY(EditDefaultsOnly, Category = "Attack")
	FGameplayTag AttackStateTag;	
	
	UPROPERTY(EditDefaultsOnly, Category = "Attack")
	TObjectPtr<UAnimMontage> AttackMontage;	
};
