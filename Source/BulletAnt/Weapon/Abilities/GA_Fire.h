#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_Fire.generated.h"

class ABaseWeapon;
class UGameplayEffect;
class URangedWeaponDataAsset;

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

	void FireOnce(const FGameplayAbilityActorInfo* ActorInfo);

	void ApplyDamageEffect(const FGameplayAbilityActorInfo* ActorInfo,
		AActor* Target,
		float Damage,
		FGameplayTag HitTag);
	
};
