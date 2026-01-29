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

protected:
	void ApplyDamage(const FGameplayAbilityActorInfo* ActorInfo, AActor* Target);

	UFUNCTION()
	void OnHitEventReceived(FGameplayEventData Payload);

	UFUNCTION()
	void OnMontageFinished();

	UPROPERTY(EditDefaultsOnly, Category = "GAS")
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	UPROPERTY(EditDefaultsOnly, Category = "GAS")
	FGameplayTag AttackTag;
};
