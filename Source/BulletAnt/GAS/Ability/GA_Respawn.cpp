#include "GAS/Ability/GA_Respawn.h"
#include "GAS/BAGameplayTags.h"
#include "Abilities/GameplayAbility.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Player/BAPlayerController.h"
#include "GAS/AttributeSet/HealthAttributeSet.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Shop/BATransportShip.h"
#include "Player/BACharacter.h"
#include "Weapon/BaseWeapon.h"

UGA_Respawn::UGA_Respawn()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;

	FAbilityTriggerData Trigger;
	Trigger.TriggerTag = TAG_Event_Combat_Dead;

	AbilityTriggers.Add(Trigger);

	BlockAbilitiesWithTag.AddTag(TAG_Ability_Active);
	ActivationOwnedTags.AddTag(TAG_State_Combat_Dead);
}

void UGA_Respawn::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	if (!GetWorld())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	Source = Cast<ABACharacter>(ActorInfo->AvatarActor.Get());
	if (!Source)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	ASC = Cast<UAbilitySystemComponent>(ActorInfo->AbilitySystemComponent.Get());
	if (!ASC)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	if (ActorInfo->IsNetAuthority())
	{
		Source->GetCharacterMovement()->DisableMovement();
		Source->GetCharacterMovement()->StopMovementImmediately();
		ASC->AddGameplayCue(TAG_GameplayCue_Combat_Dead);
	}

	GetWorld()->GetTimerManager().SetTimer(
		RespawnHandler,
		this,
		&UGA_Respawn::HandleRespawn,
		TriggerEventData->EventMagnitude,
		false
	);
}

void UGA_Respawn::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_Respawn::PreActivate(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, FOnGameplayAbilityEnded::FDelegate* OnGameplayAbilityEndedDelegate, const FGameplayEventData* TriggerEventData)
{
	ASC = ActorInfo->AbilitySystemComponent.Get();
	if (ASC)
	{
		ASC->CancelAbilities();
	}

	Super::PreActivate(Handle, ActorInfo, ActivationInfo, OnGameplayAbilityEndedDelegate, TriggerEventData);
}

void UGA_Respawn::HandleRespawn()
{
	if (Source)
	{		
		if (CurrentActorInfo->IsNetAuthority())
		{
			
			if (!TransportShipClass) return;
			FVector SpawnLocation = FVector(0.f, 0.f, 5000.f);

			ABATransportShip* Plane = GetWorld()->SpawnActor<ABATransportShip>(
				TransportShipClass,
				SpawnLocation,
				FRotator::ZeroRotator
			);
			Plane->DropFromPlane.AddDynamic(this, &UGA_Respawn::DropPlayer);

			Plane->InitPlayerPlane(SpawnLocation, Source);

			Source->AttachToComponent(
				Plane->GetMesh(),
				FAttachmentTransformRules::SnapToTargetIncludingScale,
				TEXT("SeatSocket")
			);

			Source->SetActorHiddenInGame(true);
			Source->EquippedWeapon->SetActorHiddenInGame(true);
			
			const UHealthAttributeSet* HealthSet = ASC->GetSet<UHealthAttributeSet>();

			FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(RespawnHealEffect, 1.f, ASC->MakeEffectContext());

			if (Spec.IsValid())
			{
				Spec.Data->SetSetByCallerMagnitude(
					TAG_Data_Combat_Heal,
					HealthSet->GetMaxHealth()
				);

				ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
			}
		}	
	}
}

void UGA_Respawn::DropPlayer()
{
	if (Source)
	{
		ASC->RemoveGameplayCue(TAG_GameplayCue_Combat_Dead);
		Source->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
		Source->DetachFromActor(
			FDetachmentTransformRules::KeepWorldTransform
		);
		Source->SetActorHiddenInGame(false);
		Source->EquippedWeapon->SetActorHiddenInGame(false);
	}

	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}


