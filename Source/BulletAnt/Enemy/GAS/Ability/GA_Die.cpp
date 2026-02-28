// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/GAS/Ability/GA_Die.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Components/StateTreeComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"

#include "Engine/Engine.h" // GEngine 사용을 위해 필요

UGA_Die::UGA_Die()
{
    FGameplayTag DeathTag = FGameplayTag::RequestGameplayTag(TEXT("State.Combat.Dead"));
    FGameplayTagContainer GameplayTagContainer(DeathTag);
    SetAssetTags(GameplayTagContainer);

    FAbilityTriggerData AbilityTriggerData;
    AbilityTriggerData.TriggerTag = DeathTag;
    AbilityTriggerData.TriggerSource = EGameplayAbilityTriggerSource::OwnedTagAdded;
    AbilityTriggers.Add(AbilityTriggerData);

    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;
}

void UGA_Die::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

    AActor* Avatar = GetAvatarActorFromActorInfo();
    if (!ensureMsgf(IsValid(Avatar), TEXT("GA_Die ActivateAbilitiy : Avatar Error")))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
        return;
    }

    Enemy = Cast<ABaseEnemyCharacter>(Avatar);
    if (!Enemy.IsValid())
    {
        EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
        return;
    }

    UAnimMontage* DieAnimMontage = Enemy->GetDieAnimMontage();
    if (!ensureMsgf(IsValid(DieAnimMontage), TEXT("GA_Die ActivateAbilitiy : DieAnimMontage Error")))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
        return;
    }

    UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
        this,
        NAME_None,
        DieAnimMontage
    );

    if (MontageTask)
    {
        MontageTask->OnCompleted.AddDynamic(this, &UGA_Die::OnDieAnimationFinished);
        MontageTask->OnInterrupted.AddDynamic(this, &UGA_Die::OnDieAnimationFinished);
        MontageTask->OnCancelled.AddDynamic(this, &UGA_Die::OnDieAnimationFinished);

        MontageTask->ReadyForActivation();
    }
    else
    {
        OnDieAnimationFinished();
    }
}

void UGA_Die::OnDieAnimationFinished()
{
    if (Enemy.IsValid())
    {
        if (Enemy->GetStateTreeComponent())
        {
            Enemy->GetStateTreeComponent()->StopLogic(FString());
        }
    }

    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}