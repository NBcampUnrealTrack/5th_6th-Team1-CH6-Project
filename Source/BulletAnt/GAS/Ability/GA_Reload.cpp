#include "GAS/Ability/GA_Reload.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Weapon/Data/RangedWeaponDataAsset.h"
#include "Common/DataAssetInterface.h"
#include "AbilitySystemComponent.h"
#include "GAS/BAGameplayTags.h"

UGA_Reload::UGA_Reload()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

	FGameplayTagContainer DefaultTag;
	DefaultTag.AddTag(TAG_Ability_Active_Reload);

	SetAssetTags(DefaultTag);

	ActivationOwnedTags.AddTag(TAG_State_Combat_Attacking);
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
		MontageTask->ReadyForActivation();
	}
	else
	{
		GetWorld()->GetTimerManager().SetTimer(
			ReloadHandler,
			this,
			&UGA_Reload::ReloadAmmo,
			Data->ReloadTime,
			false
		);
	}
}

void UGA_Reload::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_Reload::OnMontageFinished()
{
	ReloadAmmo();
}

void UGA_Reload::ReloadAmmo()
{
	const FGameplayAbilityActorInfo* ActorInfo = GetCurrentActorInfo();
	if (!ActorInfo || !ActorInfo->IsNetAuthority())
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	UAbilitySystemComponent* SourceASC = ActorInfo->AbilitySystemComponent.Get();
	if (!SourceASC)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
		return;
	}

	FGameplayEffectSpecHandle Spec =
		SourceASC->MakeOutgoingSpec(Data->ReloadEffect, 1.f, SourceASC->MakeEffectContext());

	if (!Spec.IsValid())
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
		return;
	}

	Spec.Data->SetSetByCallerMagnitude(
		TAG_Data_Ammo_Reload,
		Data->MaxAmmo
	);

	SourceASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());

	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
