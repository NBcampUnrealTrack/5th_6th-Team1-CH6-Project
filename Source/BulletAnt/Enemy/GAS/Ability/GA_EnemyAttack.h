// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Ability/GA_MeleeAttack.h"
#include "GA_EnemyAttack.generated.h"

/**
 * 
 */
UCLASS()
class BULLETANT_API UGA_EnemyAttack : public UGA_MeleeAttack
{
	GENERATED_BODY()
	
public:
	virtual void EndAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility,
		bool bWasCancelled) override;
};
