#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_Respawn.generated.h"

class UUW_RespawnBar;
class ABAPlayerController;
class UBAAbilitySystemComponent;
class ABaseCore;

UCLASS()
class BULLETANT_API UGA_Respawn : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_Respawn();

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
	
	virtual void PreActivate(const FGameplayAbilitySpecHandle Handle, 
		const FGameplayAbilityActorInfo* ActorInfo, 
		const FGameplayAbilityActivationInfo ActivationInfo, 
		FOnGameplayAbilityEnded::FDelegate* OnGameplayAbilityEndedDelegate,
		const FGameplayEventData* TriggerEventData = nullptr
	) override;

	void HandleRespawn();

	UPROPERTY()
	ACharacter* Source;

	UPROPERTY()
	APlayerController* PC;

	UPROPERTY()
	UUW_RespawnBar* UI;

	FTimerHandle RespawnHandler;

	FTransform SavedMeshRelativeTransform;
	UPROPERTY()
	UAbilitySystemComponent* ASC;

	FGameplayTagContainer DeadTag;
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UGameplayEffect> RespawnHealEffect;

};
