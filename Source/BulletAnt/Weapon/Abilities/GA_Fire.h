// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_Fire.generated.h"

class ABaseWeapon;
class UGameplayEffect;

UCLASS()
class BULLETANT_API UGA_Fire : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_Fire();

protected:

	UPROPERTY(EditDefaultsOnly, Category = "Fire")
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData
	) override;

	void ServerFire(const FGameplayAbilityActorInfo* ActorInfo);

	FVector GetTraceStart(const AActor* AvatarActor, ABaseWeapon* Weapon) const;
	FVector GetTraceEnd(const FVector& Start, float Range) const;
	
};
