#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_ADS.generated.h"

class ABAPlayerController;
class ABACharacter;
class ABaseWeapon;

UCLASS()
class BULLETANT_API UGA_ADS : public UGameplayAbility
{
	GENERATED_BODY()

public:

	UGA_ADS();

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
	void StartADS();

	UFUNCTION()
	void StopADS(FGameplayEventData Payload);

	FVector ADSLineTrace();

	ABAPlayerController* PC = nullptr;
	ABACharacter* Source = nullptr;
	const ABaseWeapon* CachedWeapon;
	FTransform SavedSpringArmTransform;
	FVector SavedTargetPoint;
	
};
