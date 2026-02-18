#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_Respawn.generated.h"

class USkeletalMeshComponent;

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

	UFUNCTION()
	void HandleDeath();

	UPROPERTY()
	ACharacter* Source;

	UPROPERTY()
	USkeletalMeshComponent* Mesh;

	UPROPERTY()
	APlayerController* PC;

};
