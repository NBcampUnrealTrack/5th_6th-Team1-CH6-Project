#include "GAS/Ability/GA_Respawn.h"
#include "GAS/BAGameplayTags.h"
#include "Abilities/GameplayAbility.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Player/BAPlayerController.h"
#include "GAS/AttributeSet/HealthAttributeSet.h"
#include "GAS/AbilitySystemComponent/BAAbilitySystemComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Framework/BAGameState.h"
#include "Building/BaseCore.h"

UGA_Respawn::UGA_Respawn()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;

	FAbilityTriggerData Trigger;
	Trigger.TriggerTag = TAG_Event_Combat_Dead;

	AbilityTriggers.Add(Trigger);

	DeadTag.AddTag(TAG_Event_Combat_Dead);
	bIsBlockingOtherAbilities = true;
}

void UGA_Respawn::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	if (!GetWorld()) { EndAbility(Handle, ActorInfo, ActivationInfo, true, true); return; }

	Source = Cast<ACharacter>(ActorInfo->AvatarActor.Get());
	if (!Source) { EndAbility(Handle, ActorInfo, ActivationInfo, true, true); return; }

	PC = Cast<APlayerController>(Source->GetController());
	if (PC)
	{
		Source->DisableInput(PC);
	}

	ASC = Cast<UBAAbilitySystemComponent>(ActorInfo->AbilitySystemComponent.Get());
	if (!ASC) { EndAbility(Handle, ActorInfo, ActivationInfo, true, true); return; }

	ASC->AddGameplayCue(TAG_GameplayCue_Combat_Dead);

	FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(ASC->DeadStateEffect, 1.f, ASC->MakeEffectContext());
	if (Spec.IsValid())
	{
		Spec.Data->SetSetByCallerMagnitude(
			TAG_Data_Combat_RespawnTime,
			TriggerEventData->EventMagnitude
		);

		ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data);
	}

	SavedMeshRelativeTransform = Source->GetMesh()->GetRelativeTransform();

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
	if (Source)
	{
		if (PC)
		{
			Source->EnableInput(PC);
		}

		if (ASC)
		{
			ASC->RemoveGameplayCue(TAG_GameplayCue_Combat_Dead);

			const UHealthAttributeSet* HealthSet = ASC->GetSet<UHealthAttributeSet>();
			FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(ASC->RespawnHealEffect, 1.f, ASC->MakeEffectContext());

			if (Spec.IsValid() && HealthSet)
			{
				Spec.Data->SetSetByCallerMagnitude(
					TAG_Data_Combat_Heal,
					HealthSet->GetMaxHealth()
				);
				ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
			}
		}

		Source->GetMesh()->SetRelativeTransform(SavedMeshRelativeTransform);
		Source->TeleportTo(FVector(0.f, 0.f, 5.f), FRotator::ZeroRotator, false, true);
		Source->ForceNetUpdate();
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_Respawn::HandleRespawn()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}


