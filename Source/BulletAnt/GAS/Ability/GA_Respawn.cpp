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

UGA_Respawn::UGA_Respawn()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;

	FAbilityTriggerData Trigger;
	Trigger.TriggerTag = TAG_Event_Combat_Dead;

	AbilityTriggers.Add(Trigger);
	bIsBlockingOtherAbilities = true;
}

void UGA_Respawn::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	if (!GetWorld()) return;

	Source = Cast<ACharacter>(ActorInfo->AvatarActor.Get());
	if (!Source) return;

	PC = Cast<APlayerController>(Source->GetController());
	if (PC)
	{
		Source->DisableInput(PC);
	}

	UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
	if (!ASC) return;

	ActorInfo->AbilitySystemComponent->AddGameplayCue(TAG_GameplayCue_Combat_Dead);
	ASC->AddLooseGameplayTag(TAG_State_Combat_Dead);

	UKismetSystemLibrary::PrintString(GetWorld(), FString("Hello World"));

	Source->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_None);
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
	if (!Source) return;
	if (PC)
	{
		Source->EnableInput(PC);
	}

	ActorInfo->AbilitySystemComponent->RemoveGameplayCue(TAG_GameplayCue_Combat_Dead);
	
	Source->GetMesh()->SetRelativeTransform(SavedMeshRelativeTransform);

	Source->TeleportTo(FVector(0.f, 0.f, 5.f), FRotator::ZeroRotator, false, true);

	Source->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);

	UBAAbilitySystemComponent* ASC = Cast<UBAAbilitySystemComponent>(ActorInfo->AbilitySystemComponent.Get());
	ASC->RemoveLooseGameplayTag(TAG_State_Combat_Dead);

	const UHealthAttributeSet* HealthSet = ASC->GetSet<UHealthAttributeSet>();

	FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(ASC->RespawnHealEffect, 1.f, ASC->MakeEffectContext());

	if (!Spec.IsValid()) return;

	Spec.Data->SetSetByCallerMagnitude(
		TAG_Data_Combat_Heal,
		HealthSet->GetMaxHealth()
	);

	ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_Respawn::HandleRespawn()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}


