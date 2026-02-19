#include "GAS/Ability/GA_SwitchWeapon.h"
#include "Weapon/BaseWeapon.h"
#include "Player/BACharacter.h"
#include "AbilitySystemComponent.h"
#include "GAS/BAGameplayTags.h"

UGA_SwitchWeapon::UGA_SwitchWeapon()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;

	FAbilityTriggerData TriggerData;
	TriggerData.TriggerTag = TAG_Event_Weapon_Switch;
	TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;

	AbilityTriggers.Add(TriggerData);
}

void UGA_SwitchWeapon::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle, 
	const FGameplayAbilityActorInfo* ActorInfo, 
	const FGameplayAbilityActivationInfo ActivationInfo, 
	const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}
	
	ApplySwitchEffect(ActorInfo, TriggerEventData);

	EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
}

void UGA_SwitchWeapon::ApplySwitchEffect(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayEventData* TriggerEventData)
{
	UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
	const ABaseWeapon* Weapon = Cast<ABaseWeapon>(TriggerEventData->OptionalObject);
	if (!Weapon || !ASC) return;

	ASC->RemoveActiveEffectsWithAppliedTags(FGameplayTagContainer(FGameplayTag::RequestGameplayTag("Weapon.Equipped")));

	SwitchEffectClass = Weapon->GetSwitchEffectClass();
	if (!SwitchEffectClass) return;

	FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
	Context.AddSourceObject(Weapon);

	FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(SwitchEffectClass, 1.0f, Context);
	if (SpecHandle.IsValid())
	{
		ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
	}
}
