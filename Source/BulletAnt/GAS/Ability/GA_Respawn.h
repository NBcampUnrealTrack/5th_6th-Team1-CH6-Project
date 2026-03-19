#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_Respawn.generated.h"

class UUW_RespawnBar;
class ABAPlayerController;
class UBAAbilitySystemComponent;
class ABaseCore;
class ABATransportShip;
class ABACharacter;

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

	UFUNCTION()
	void DropPlayer();

	UPROPERTY()
	ABACharacter* Source;

	UPROPERTY()
	UUW_RespawnBar* UI;

	FTimerHandle RespawnHandler;
	UPROPERTY()
	UAbilitySystemComponent* ASC;

	FGameplayTagContainer DeadTag;
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UGameplayEffect> RespawnHealEffect;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<ABATransportShip> TransportShipClass;
};
