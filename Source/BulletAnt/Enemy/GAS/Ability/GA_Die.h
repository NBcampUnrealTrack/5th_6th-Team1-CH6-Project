// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "Enemy/BaseEnemy/BaseEnemyCharacter.h"
#include "GA_Die.generated.h"

/**
 * 
 */
UCLASS()
class BULLETANT_API UGA_Die : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_Die();

	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData
	) override;	

	UFUNCTION()
	void OnDieAnimationFinished();

private:
	TWeakObjectPtr<ABaseEnemyCharacter> Enemy;
};
