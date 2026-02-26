#pragma once

#include "CoreMinimal.h"
#include "Weapon/Abilities/GA_Fire.h"
#include "GA_PlayerFire.generated.h"

class ABACharacter;

UCLASS()
class BULLETANT_API UGA_PlayerFire : public UGA_Fire
{
	GENERATED_BODY()

	UGA_PlayerFire();

	virtual void FireOnce(const FGameplayAbilityActorInfo* ActorInfo) override;

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

	UPROPERTY()
	TObjectPtr<ABACharacter> PlayerCharacter;

	float RecoilPitch;
	float RecoilYaw;
};
