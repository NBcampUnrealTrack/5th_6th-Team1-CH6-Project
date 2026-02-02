#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_Mine.generated.h"

UCLASS()
class BULLETANT_API UGA_Mine : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_Mine();

	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData
	) override;

	void MiningOnce(const FGameplayAbilityActorInfo* ActorInfo);

	void ApplyMiningEffect(const FGameplayAbilityActorInfo* ActorInfo,
		AActor* Target,
		float Damage,
		FGameplayTag HitTag);
};
