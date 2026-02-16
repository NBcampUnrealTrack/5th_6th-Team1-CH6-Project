#include "GAS/Ability/GA_Reload.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Weapon/Data/RangedWeaponDataAsset.h"
#include "Common/DataAssetInterface.h"
#include "AbilitySystemComponent.h"

UGA_Reload::UGA_Reload()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

	AbilityTags.AddTag(FGameplayTag::RequestGameplayTag(TEXT("Ability.Active.Reload")));

	TAG_Data_Ammo_Reload = FGameplayTag::RequestGameplayTag(TEXT("Data.Ammo.Reload"));
}

void UGA_Reload::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	IDataAssetInterface* Interface = Cast<IDataAssetInterface>(GetAvatarActorFromActorInfo());
	if (!Interface || !Interface->GetDataAsset())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	SourceActor = Cast<AActor>(ActorInfo->AvatarActor);
	if (!SourceActor) return;

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

void UGA_Reload::OnMontageFinished()
{
	ReloadAmmo();
}

void UGA_Reload::ReloadAmmo()
{
	const FGameplayAbilityActorInfo* ActorInfo = GetCurrentActorInfo();
	if (!ActorInfo || !ActorInfo->IsNetAuthority()) return;

	UAbilitySystemComponent* SourceASC = ActorInfo->AbilitySystemComponent.Get();
	if (!SourceASC) return;

	FGameplayEffectSpecHandle Spec =
		SourceASC->MakeOutgoingSpec(Data->ReloadEffect, 1.f, SourceASC->MakeEffectContext());

	if (!Spec.IsValid()) return;

	Spec.Data->SetSetByCallerMagnitude(
		TAG_Data_Ammo_Reload,
		Data->MaxAmmo
	);

	SourceASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
}
