#include "GAS/Ability/GA_Respawn.h"
#include "GAS/BAGameplayTags.h"
#include "Abilities/GameplayAbility.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AbilitySystemComponent.h"

UGA_Respawn::UGA_Respawn()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;

	FAbilityTriggerData Trigger;
	Trigger.TriggerTag = TAG_State_Combat_Dead;

	AbilityTriggers.Add(Trigger);
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

	Mesh = Source->GetMesh();
	if (!Mesh) return;

	PC = Cast<APlayerController>(Source->GetController());
	if (!PC) return;

	Source->DisableInput(PC);
	Source->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_None);
	Mesh->SetVisibility(false);

	FTimerHandle DeathTimer;

	GetWorld()->GetTimerManager().SetTimer(
		DeathTimer,
		this,
		&UGA_Respawn::HandleDeath,
		TriggerEventData->EventMagnitude,
		false
	);
}

void UGA_Respawn::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (!Source) return;
	if (!PC) return;
	if (!Mesh) return;

	Source->EnableInput(PC);
	Source->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
	Mesh->SetVisibility(true);

	UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
	ASC->AddLooseGameplayTag(TAG_State_Combat_Dead);

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_Respawn::HandleDeath()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
