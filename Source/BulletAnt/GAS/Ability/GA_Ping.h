#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_Ping.generated.h"

UCLASS()
class BULLETANT_API UGA_Ping : public UGameplayAbility
{
	GENERATED_BODY()
	
public:
	UGA_Ping();

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
	void SendTargetData();
	void OnTargetDataReady(const FGameplayAbilityTargetDataHandle& Data, FGameplayTag Tag);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TraceDistance = 1600.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TraceRadius = 30.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UGameplayEffect> GE_Ping;
};
