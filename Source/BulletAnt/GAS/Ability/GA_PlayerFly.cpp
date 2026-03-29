#include "GAS/Ability/GA_PlayerFly.h"

#include "Common/DataAssetInterface.h"
#include "Weapon/Data/JetpackWeaponDataAsset.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AbilitySystemComponent.h"
#include "GAS/BAGameplayTags.h"
#include "Weapon/BaseJetpackWeapon.h"
#include "Player/BACharacter.h"

UGA_PlayerFly::UGA_PlayerFly()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

	FGameplayTagContainer DefaultTag;
	DefaultTag.AddTag(TAG_Ability_Active_Jetpack);

	SetAssetTags(DefaultTag);

	ActivationOwnedTags.AddTag(TAG_State_Combat_Attacking);
}

void UGA_PlayerFly::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	AActor* SourceActor = Cast<AActor>(ActorInfo->AvatarActor);
	if (!SourceActor)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	PlayerCharacter = Cast<ABACharacter>(SourceActor);
	if (!PlayerCharacter)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	IDataAssetInterface* DataAssetInterface = Cast<IDataAssetInterface>(SourceActor);
	if (!DataAssetInterface)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	UJetpackWeaponDataAsset* Data = Cast<UJetpackWeaponDataAsset>(DataAssetInterface->GetDataAsset());
	if (!Data)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	PlayerCharacter->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Flying);

	if (ActorInfo->IsNetAuthority())
	{
		ABaseJetpackWeapon* Jetpack = Cast<ABaseJetpackWeapon>(PlayerCharacter->EquippedWeapon);
		if (Jetpack)
		{
			Jetpack->SetbJetpackActive(true);
			Jetpack->OnRep_bJetpackActive();			
		}
	}

	if (Data->bAutoActive)
	{
		LoopInputUpMovement();
	}
	else
	{
		InputUpMovementOnce();
	}
}

void UGA_PlayerFly::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	GetWorld()->GetTimerManager().ClearTimer(FlyTimerHandler);
	
	PlayerCharacter->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
	if (ActorInfo->IsNetAuthority())
	{
		ABaseJetpackWeapon* Jetpack = Cast<ABaseJetpackWeapon>(PlayerCharacter->EquippedWeapon);
		if (Jetpack)
		{
			Jetpack->SetbJetpackActive(false);
			Jetpack->OnRep_bJetpackActive();			
		}
	}


	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_PlayerFly::LoopInputUpMovement()
{
	GetWorld()->GetTimerManager().SetTimer(
		FlyTimerHandler,
		this,
		&UGA_PlayerFly::InputUpMovementOnce,
		0.1f,
		true,
		-1.0f
	);
}

void UGA_PlayerFly::InputUpMovementOnce()
{
	if (!PlayerCharacter) return;

	PlayerCharacter->AddMovementInput(FVector::UpVector, 1.f);
}
