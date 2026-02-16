#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_PlayerFly.generated.h"

UCLASS()
class BULLETANT_API UGA_PlayerFly : public UGameplayAbility
{
	GENERATED_BODY()
	
public:
	UGA_PlayerFly();

protected:
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

	UFUNCTION()
	void LoopInputUpMovement();

	void InputUpMovementOnce();

	UPROPERTY()
	ACharacter* PlayerCharacter;

	UPROPERTY()
	float CachedZSpeed;

	FTimerHandle FlyTimerHandler;
	FActiveGameplayEffectHandle FlyingStateHandle;
};
