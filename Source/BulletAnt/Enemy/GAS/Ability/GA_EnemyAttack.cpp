// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/GAS/Ability/GA_EnemyAttack.h"
#include "Enemy/BaseEnemy/BaseEnemyCharacter.h"
#include "Components/StateTreeComponent.h"

UGA_EnemyAttack::UGA_EnemyAttack()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;
}

void UGA_EnemyAttack::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	AActor* Avatar = CurrentActorInfo->AvatarActor.Get();
	if (IsValid(Avatar))
	{
		ABaseEnemyCharacter* Enemy = Cast<ABaseEnemyCharacter>(Avatar);
		if (IsValid(Enemy))
		{
			if (Enemy->ShouldCallAfterAttack())
			{
				Enemy->AfterAttack();
			}
			else
			{
				FStateTreeEvent ToRotate(FGameplayTag::RequestGameplayTag(TEXT("State.Movement.Rotating")));
				Enemy->GetStateTreeComponent()->SendStateTreeEvent(ToRotate);
			}
		}
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
