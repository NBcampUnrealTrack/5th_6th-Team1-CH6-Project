#include "GAS/Ability/GA_MeleeAttack.h"
#include "Common/DataAssetInterface.h"
#include "Weapon/Data/MeleeWeaponDataAsset.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GAS/BAGameplayTags.h"


UGA_MeleeAttack::UGA_MeleeAttack()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

	FGameplayTagContainer DefaultTag;
	DefaultTag.AddTag(TAG_Ability_Active_MeleeAttack);

	SetAssetTags(DefaultTag);

	BlockAbilitiesWithTag.AddTag(TAG_Ability_Active);

	ActivationOwnedTags.AddTag(TAG_State_Combat_Attacking);
}

void UGA_MeleeAttack::ActivateAbility(
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

	IDataAssetInterface* Interface = Cast<IDataAssetInterface>(GetAvatarActorFromActorInfo());
	if (!Interface || !Interface->GetDataAsset())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	UMeleeWeaponDataAsset* Data = Cast<UMeleeWeaponDataAsset>(Interface->GetDataAsset());

	UAbilityTask_WaitGameplayEvent* WaitTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, Data->HitEventTag);
	WaitTask->EventReceived.AddDynamic(this, &UGA_MeleeAttack::OnHitEventReceived);
	WaitTask->ReadyForActivation();

	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, Data->AttackMontage);
	MontageTask->OnCompleted.AddDynamic(this, &UGA_MeleeAttack::OnMontageFinished);
	MontageTask->OnInterrupted.AddDynamic(this, &UGA_MeleeAttack::OnMontageFinished);
	MontageTask->ReadyForActivation();
}

void UGA_MeleeAttack::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_MeleeAttack::OnHitEventReceived(FGameplayEventData Payload)
{
	if(!CurrentActorInfo->IsNetAuthority()) return;

	AActor* HitActor = const_cast<AActor*>(Payload.Target.Get());

	if (!HitActor)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	ApplyDamage(CurrentActorInfo, HitActor);
}

void UGA_MeleeAttack::OnMontageFinished()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}


void UGA_MeleeAttack::ApplyDamage(const FGameplayAbilityActorInfo* ActorInfo, AActor* Target)
{
	if (!ActorInfo || !Target)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	UAbilitySystemComponent* SourceASC = ActorInfo->AbilitySystemComponent.Get();
	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Target);

	if (SourceASC && TargetASC)
	{
		IDataAssetInterface* Interface = Cast<IDataAssetInterface>(ActorInfo->AvatarActor.Get());
		if (!Interface || !Interface->GetDataAsset())
		{
			EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
			return;
		}

		UMeleeWeaponDataAsset* Data = Cast<UMeleeWeaponDataAsset>(Interface->GetDataAsset());
		if (!Data)
		{
			EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
			return;
		}

		FGameplayEffectContextHandle ContextHandle = SourceASC->MakeEffectContext();
		ContextHandle.AddInstigator(ActorInfo->OwnerActor.Get(), ActorInfo->AvatarActor.Get());

		if (Data->OnUseStateHitEffect)
		{
			FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(Data->OnUseStateHitEffect, GetAbilityLevel(), ContextHandle);

			if (SpecHandle.IsValid())
			{
				SpecHandle.Data->SetSetByCallerMagnitude(TAG_Data_Combat_Damage, Data->BaseDamage);
				SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
			}
		}
	}
}
