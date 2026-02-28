

#include "GAS/Ability/GA_DestroyBuilding.h"
#include "GAS/BAGameplayTags.h"
#include "Abilities/GameplayAbility.h"
#include "Building/BaseBuilding.h"

UGA_DestroyBuilding::UGA_DestroyBuilding()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;

	FAbilityTriggerData Trigger;
	Trigger.TriggerTag = TAG_Event_Combat_Dead;

	AbilityTriggers.Add(Trigger);
}

void UGA_DestroyBuilding::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	if (ABaseBuilding* Source = Cast<ABaseBuilding>(ActorInfo->AvatarActor.Get()))
	{
		Source->OnDeath();
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
