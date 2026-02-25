#include "GAS/Ability/GA_PlayerFire.h"
#include "Player/BACharacter.h"
#include "Player/BAPlayerController.h"
#include "Weapon/Data/RangedWeaponDataAsset.h"
#include "GAS/BAGameplayTags.h"

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
	if (!PlayerCharacter) return;

	IDataAssetInterface* DataAssetInterface = Cast<IDataAssetInterface>(PlayerCharacter);
	if (!DataAssetInterface) return;

	RangedData = Cast<URangedWeaponDataAsset>(DataAssetInterface->GetDataAsset());
	if (!RangedData) return;
	
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
