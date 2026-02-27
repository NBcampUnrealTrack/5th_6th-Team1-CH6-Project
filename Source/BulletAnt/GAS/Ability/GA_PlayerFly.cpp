#include "GAS/Ability/GA_PlayerFly.h"

#include "Common/DataAssetInterface.h"
#include "Weapon/Data/JetpackWeaponDataAsset.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AbilitySystemComponent.h"
#include "GAS/BAGameplayTags.h"

UGA_PlayerFly::UGA_PlayerFly()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

	FGameplayTagContainer DefaultTag;
	DefaultTag.AddTag(TAG_Ability_Active_Jetpack);

	SetAssetTags(DefaultTag);
}

void UGA_PlayerFly::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	AActor* SourceActor = Cast<AActor>(ActorInfo->AvatarActor);
	if (!SourceActor) return;

	PlayerCharacter = Cast<ACharacter>(SourceActor);
	if (!PlayerCharacter) return;

	IDataAssetInterface* DataAssetInterface = Cast<IDataAssetInterface>(SourceActor);
	if (!DataAssetInterface) return;

	UJetpackWeaponDataAsset* Data = Cast<UJetpackWeaponDataAsset>(DataAssetInterface->GetDataAsset());
	if (!Data) return;

	PlayerCharacter->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Flying);
	
	if (Data->UseStateEffect)
	{
		const UGameplayEffect* EffectCDO = Data->UseStateEffect->GetDefaultObject<UGameplayEffect>();
		FlyingStateHandle = ApplyGameplayEffectToOwner(Handle, ActorInfo, ActivationInfo, EffectCDO, 1.f, 1);
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

	if (FlyingStateHandle.IsValid())
	{
		GetAbilitySystemComponentFromActorInfo()->RemoveActiveGameplayEffect(FlyingStateHandle);
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

	PlayerCharacter->AddMovementInput(FVector::UpVector, 1.0f);
}
