#include "GAS/Ability/GA_PlayerFire.h"
#include "Player/BACharacter.h"
#include "Player/BAPlayerController.h"
#include "Weapon/Data/RangedWeaponDataAsset.h"
#include "GAS/BAGameplayTags.h"
#include "Player/BAAnimInstance.h"
#include "GAS/AttributeSet/AmmoAttributeSet.h"

UGA_PlayerFire::UGA_PlayerFire()
{
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

void UGA_PlayerFire::FireOnce()
{
	const UAmmoAttributeSet* AmmoSet = GetAbilitySystemComponentFromActorInfo()->GetSet<UAmmoAttributeSet>();
	if (AmmoSet)
	{
		if (AmmoSet->GetCurrentAmmo() <= 0.f)
		{
			EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
			return;
		}
	}

	// 장탄수 관리
	if (CurrentActorInfo->IsNetAuthority())
	{
		if (CostEffect)
		{
			FGameplayEffectContextHandle Context = CachedASC->MakeEffectContext();

			FGameplayEffectSpecHandle SpecHandle = CachedASC->MakeOutgoingSpec(CostEffect, 1.0f, Context);
			if (SpecHandle.IsValid())
			{
				CachedASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
			}
		}
	}

	if (IsLocallyControlled())
	{
		float InRecoilPitch = RecoilPitch;
		float InRecoilYaw = FMath::RandRange(-RecoilYaw, RecoilYaw);

		if (PlayerCharacter)
		{
			PlayerCharacter->SetRecoil(InRecoilPitch, InRecoilYaw);
		}
	}

	Super::FireOnce();
}

void UGA_PlayerFire::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	PlayerCharacter = Cast<ABACharacter>(ActorInfo->AvatarActor);
	if (!PlayerCharacter)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	IDataAssetInterface* DataAssetInterface = Cast<IDataAssetInterface>(PlayerCharacter);
	if (!DataAssetInterface)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	RangedData = Cast<URangedWeaponDataAsset>(DataAssetInterface->GetDataAsset());
	if (!RangedData)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	PlayerCharacter->SetbIsFiring(true);

	if (IsLocallyControlled())
	{
		if (RangedData && RangedData->bPlayer)
		{
			RecoilPitch = RangedData->RecoilPitch;
			RecoilYaw = RangedData->RecoilYaw;
		}
	}
}

void UGA_PlayerFire::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	//쿨타임 관리
	if (ActorInfo->IsNetAuthority())
	{
		FGameplayEffectContextHandle Context = CachedASC->MakeEffectContext();

		FGameplayEffectSpecHandle SpecHandle = CachedASC->MakeOutgoingSpec(CooldownEffect, 1.0f, Context);
		if (SpecHandle.IsValid())
		{
			SpecHandle.Data->SetSetByCallerMagnitude(
				TAG_Data_Fire_Cooldown,
				FireDelay
			);

			CachedASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
		}
	}

	if (PlayerCharacter)
	{
		PlayerCharacter->SetbIsFiring(false);
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
