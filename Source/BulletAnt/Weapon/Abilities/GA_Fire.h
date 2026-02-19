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

	void FireOnce(const FGameplayAbilityActorInfo* ActorInfo);

	void StartAutoFireLoop();

	void ApplyDamageEffect(const FGameplayAbilityActorInfo* ActorInfo,
		AActor* Target,
		const URangedWeaponDataAsset* WeaponData);

	UFUNCTION()
	FVector ApplySpread(const FVector& Dir, float Degree);

	FActiveGameplayEffectHandle AttackingStateHandle;

	UPROPERTY()
	TObjectPtr<AActor> SourceActor;

	UPROPERTY()
	URangedWeaponDataAsset* RangedData;

	UPROPERTY()
	FTimerHandle FireTimerHandler;

	UPROPERTY()
	UAbilitySystemComponent* CachedASC;

	int32 ContinuousBullet;
	float CurrentRecoilPitch;
	float CurrentRecoilYaw;
};
