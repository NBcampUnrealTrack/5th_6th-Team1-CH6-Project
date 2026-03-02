#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_Reload.generated.h"

class URangedWeaponDataAsset;

UCLASS()
class BULLETANT_API UGA_Reload : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_Reload();

	

public:

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
	void OnMontageFinished();

	UFUNCTION()
	void ReloadAmmo();

	UPROPERTY()
	URangedWeaponDataAsset* Data;

	UPROPERTY()
	AActor* SourceActor;

	FTimerHandle ReloadHandler;
	
};
