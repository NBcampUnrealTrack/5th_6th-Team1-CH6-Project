
#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_DestroyBuilding.generated.h"

UCLASS()
class BULLETANT_API UGA_DestroyBuilding : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_DestroyBuilding();

protected:
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData
	) override;
};
