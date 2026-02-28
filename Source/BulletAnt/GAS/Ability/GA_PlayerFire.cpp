#include "GAS/Ability/GA_PlayerFire.h"
#include "Player/BACharacter.h"
#include "Player/BAPlayerController.h"
#include "Weapon/Data/RangedWeaponDataAsset.h"
#include "GAS/BAGameplayTags.h"
#include "Player/BAAnimInstance.h"

UGA_PlayerFire::UGA_PlayerFire()
{
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

void UGA_PlayerFire::FireOnce(const FGameplayAbilityActorInfo* ActorInfo)
{
	Super::FireOnce(ActorInfo);

	if (IsLocallyControlled())
	{
		float InRecoilPitch = FMath::RandRange(0.f, RecoilPitch);
		float InRecoilYaw = FMath::RandRange(-RecoilYaw, RecoilYaw);

		if (PlayerCharacter)
		{
			PlayerCharacter->SetRecoil(InRecoilPitch, InRecoilYaw);
		}
	}
}

void UGA_PlayerFire::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	PlayerCharacter = Cast<ABACharacter>(ActorInfo->AvatarActor);
	if (!PlayerCharacter) { EndAbility(Handle, ActorInfo, ActivationInfo, true, true); return; }

	IDataAssetInterface* DataAssetInterface = Cast<IDataAssetInterface>(PlayerCharacter);
	if (!DataAssetInterface) { EndAbility(Handle, ActorInfo, ActivationInfo, true, true); return; }

	RangedData = Cast<URangedWeaponDataAsset>(DataAssetInterface->GetDataAsset());
	if (!RangedData) { EndAbility(Handle, ActorInfo, ActivationInfo, true, true); return; }

	UBAAnimInstance* Anim = Cast<UBAAnimInstance>(PlayerCharacter->GetMesh()->GetAnimInstance());
	if (Anim)
	{
		Anim->SetIsFiring(true);
	}
	
	if (IsLocallyControlled()) 
	{
		if (RangedData && RangedData->bPlayer)
		{
			RecoilPitch = RangedData->RecoilPitch;
			RecoilYaw = RangedData->RecoilYaw;
		}
	}

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UGA_PlayerFire::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (PlayerCharacter)
	{
		UBAAnimInstance* Anim = Cast<UBAAnimInstance>(PlayerCharacter->GetMesh()->GetAnimInstance());
		if (Anim)
		{
			Anim->SetIsFiring(false);
		}
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
