#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GameplayEffect.h"
#include "GA_SwitchWeapon.generated.h"

UCLASS()
class BULLETANT_API UGA_SwitchWeapon : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_SwitchWeapon();

	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData
	) override;

protected:

	void ApplySwitchEffect(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayEventData* TriggerEventData);

	UPROPERTY()
	TSubclassOf<UGameplayEffect> SwitchEffectClass;


	
};
