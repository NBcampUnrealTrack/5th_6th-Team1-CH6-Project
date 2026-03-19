#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_MeleeAttack.generated.h"

class ABaseWeapon;
class UGameplayEffect;
class UMeleeWeaponDataAsset;

UCLASS()
class BULLETANT_API UGA_MeleeAttack : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_MeleeAttack();

	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData
	) override;

	virtual void EndAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility,
		bool bWasCancelled) override;

protected:
	void ApplyDamage(const FGameplayAbilityActorInfo* ActorInfo, AActor* Target);

	UFUNCTION()
	void OnStartEventReceived(FGameplayEventData Payload);

	UFUNCTION()
	void OnEndEventReceived(FGameplayEventData Payload);

	UFUNCTION()
	void OnMontageFinished();

	void PerformHitCheck();

	TArray<AActor*> HitActors;

	UMeleeWeaponDataAsset* Data;

	ACharacter* OwnerActor;

	FTimerHandle HitCheckTimer;
};
