#include "GAS/Ability/GA_MeleeAttack.h"
#include "Weapon/BaseWeapon.h"
#include "Weapon/Data/MeleeWeaponDataAsset.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"

static const FGameplayTag TAG_Data_Combat_Damage = FGameplayTag::RequestGameplayTag(TEXT("Data.Combat.Damage"));

UGA_MeleeAttack::UGA_MeleeAttack()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;

	AbilityTags.AddTag(FGameplayTag::RequestGameplayTag(TEXT("Ability.Weapon.MeleeAttack")));
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

	ABaseWeapon* SourceWeapon = Cast<ABaseWeapon>(GetCurrentSourceObject());
	if (!SourceWeapon)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	UMeleeWeaponDataAsset* Data = Cast<UMeleeWeaponDataAsset>(SourceWeapon->GetWeaponData());
	if (!Data)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	UAbilityTask_WaitGameplayEvent* WaitTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, Data->HitEventTag);
	WaitTask->EventReceived.AddDynamic(this, &UGA_MeleeAttack::OnHitEventReceived);
	WaitTask->ReadyForActivation();

	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, Data->AttackMontage);
	MontageTask->OnCompleted.AddDynamic(this, &UGA_MeleeAttack::OnMontageFinished);
	MontageTask->OnInterrupted.AddDynamic(this, &UGA_MeleeAttack::OnMontageFinished);
	MontageTask->ReadyForActivation();
}

void UGA_MeleeAttack::OnHitEventReceived(FGameplayEventData Payload)
{
	AActor* HitActor = const_cast<AActor*>(Payload.Target.Get());

	if (!HitActor)
	{
		return;
	}
	UE_LOG(LogTemp, Log, TEXT("Melee Hit Success: %s"), *HitActor->GetName());

	ApplyDamage(CurrentActorInfo, HitActor);
}

void UGA_MeleeAttack::OnMontageFinished()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}


void UGA_MeleeAttack::ApplyDamage(const FGameplayAbilityActorInfo* ActorInfo, AActor* Target)
{
	if (!ActorInfo || !Target) return;

	UAbilitySystemComponent* SourceASC = ActorInfo->AbilitySystemComponent.Get();
	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Target);

	if (!SourceASC || !TargetASC) return;

	ABaseWeapon* SourceWeapon = Cast<ABaseWeapon>(GetCurrentSourceObject());
	if (!SourceWeapon) return;

	UMeleeWeaponDataAsset* Data = Cast<UMeleeWeaponDataAsset>(SourceWeapon->GetWeaponData());
	if (!Data) return;

	FGameplayEffectContextHandle ContextHandle = SourceASC->MakeEffectContext();
	ContextHandle.AddInstigator(ActorInfo->OwnerActor.Get(), ActorInfo->AvatarActor.Get());

	if (DamageEffectClass)
	{
		FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(DamageEffectClass, GetAbilityLevel(), ContextHandle);

		if (SpecHandle.IsValid())
		{
			SpecHandle.Data->SetSetByCallerMagnitude(TAG_Data_Combat_Damage, Data->BaseDamage);
			SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
		}
	}
}

