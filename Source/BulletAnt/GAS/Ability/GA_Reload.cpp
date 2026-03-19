#include "GAS/Ability/GA_Reload.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Weapon/Data/RangedWeaponDataAsset.h"
#include "Common/DataAssetInterface.h"
#include "AbilitySystemComponent.h"
#include "GAS/BAGameplayTags.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"

UGA_Reload::UGA_Reload()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

	FGameplayTagContainer DefaultTag;
	DefaultTag.AddTag(TAG_Ability_Active_Reload);

	SetAssetTags(DefaultTag);

	ActivationOwnedTags.AddTag(TAG_State_Combat_Reload);
	BlockAbilitiesWithTag.AddTag(TAG_Ability_Active_Fire);
}

void UGA_Reload::PreActivate(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, FOnGameplayAbilityEnded::FDelegate* OnGameplayAbilityEndedDelegate, const FGameplayEventData* TriggerEventData)
{
	UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
	FGameplayTagContainer IgnoreTag;
	IgnoreTag.AddTag(TAG_Event_Combat_Dead);

	ASC->CancelAbilities(nullptr, &IgnoreTag);

	Super::PreActivate(Handle, ActorInfo, ActivationInfo, OnGameplayAbilityEndedDelegate, TriggerEventData);
}

void UGA_Reload::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	IDataAssetInterface* Interface = Cast<IDataAssetInterface>(GetAvatarActorFromActorInfo());
	if (!Interface || !Interface->GetDataAsset())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	SourceActor = Cast<AActor>(ActorInfo->AvatarActor);
	if (!SourceActor)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	Data = Cast<URangedWeaponDataAsset>(Interface->GetDataAsset());

	if (Data->ReloadMontage)
	{
		UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, Data->ReloadMontage);
		MontageTask->OnCompleted.AddDynamic(this, &UGA_Reload::OnMontageFinished);
		MontageTask->OnCancelled.AddDynamic(this, &UGA_Reload::OnFailedMontage);
		MontageTask->OnInterrupted.AddDynamic(this, &UGA_Reload::OnFailedMontage);
		MontageTask->ReadyForActivation();
	}
	else
	{
		UAbilityTask_WaitDelay* DelayTask = UAbilityTask_WaitDelay::WaitDelay(this, 2.f);

		DelayTask->OnFinish.AddDynamic(this, &UGA_Reload::ReloadAmmo);
		DelayTask->ReadyForActivation();
	}
}

void UGA_Reload::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_Reload::OnFailedMontage()
{
	CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
}

void UGA_Reload::OnMontageFinished()
{
	ReloadAmmo();
}

void UGA_Reload::ReloadAmmo()
{
	const FGameplayAbilityActorInfo* ActorInfo = GetCurrentActorInfo();
	if (ActorInfo->IsNetAuthority())
	{
		UAbilitySystemComponent* SourceASC = ActorInfo->AbilitySystemComponent.Get();
		if (!SourceASC)
		{
			EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
			return;
		}

		FGameplayEffectSpecHandle Spec =
			SourceASC->MakeOutgoingSpec(Data->ReloadEffect, 1.f, SourceASC->MakeEffectContext());

		if (Spec.IsValid())
		{
			Spec.Data->SetSetByCallerMagnitude(
				TAG_Data_Ammo_Reload,
				Data->MaxAmmo
			);

			SourceASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
		}

		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
	}
}