#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_Mine.generated.h"

class UMiningWeaponDataAsset;

UCLASS()
class BULLETANT_API UGA_Mine : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_Mine();

	void StartAutoDigLoop();

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
	virtual void CancelAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateCancelAbility) override;

	void MiningOnce();
	void EndMining();

	UFUNCTION()
	void OnMontageFinished();

protected:
	UPROPERTY()
	float Playrate;
	float TargetDuration;

	UPROPERTY()
	TObjectPtr<AActor> SourceActor;

	UPROPERTY()
	UMiningWeaponDataAsset* MiningData;

	UPROPERTY()
	FTimerHandle DigTimerHandler;

	UPROPERTY()
	UAnimMontage* CachedMiningAM;

	bool bEndInput;
};
