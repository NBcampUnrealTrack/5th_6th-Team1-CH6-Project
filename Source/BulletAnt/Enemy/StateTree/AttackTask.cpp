// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/StateTree/AttackTask.h"
#include "Enemy/BaseEnemy/BaseEnemyCharacter.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Enemy/DataAsset/BaseEnemyDataAsset.h"
#include "GameplayTagContainer.h"
#include "Weapon/Data/WeaponDataAsset.h"
#include "AIController.h"

EStateTreeRunStatus UAttackTask::EnterState(FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition)
{
	Super::EnterState(Context, Transition);
	
	if (!IsValid(ContextActor))
	{
		UE_LOG(LogTemp, Error, TEXT("UAttackTask-EnterState : ContextActor Error"));
		// 해당 몬스터 제거 후 다시 스폰
		return EStateTreeRunStatus::Failed;
	}

	CachedAIController = ContextActor->GetController<AAIController>();
	if (!ensureMsgf(CachedAIController.IsValid(), TEXT("AttackTask-EnterState : AIController Error")))
	{
		return EStateTreeRunStatus::Failed;
	}
	
	if (!ensureMsgf(IsValid(ContextActor->BaseEnemyDataAsset), TEXT("UAttackTask-EnterState : DataAsset Error")))
	{
		return EStateTreeRunStatus::Failed;
	}
	if (UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(ContextActor))
	{
		UWeaponDataAsset* WeaponDataAsset = Cast<UWeaponDataAsset>(ContextActor->GetDataAsset());
		if (IsValid(WeaponDataAsset))
		{
			if (CachedAIController.IsValid())
			{
				CachedAIController->ClearFocus(EAIFocusPriority::Gameplay);
			}
			ASC->TryActivateAbilitiesByTag(FGameplayTagContainer(WeaponDataAsset->WeaponTag));
		}
	}
	
	return EStateTreeRunStatus::Running;
}

void UAttackTask::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition)
{	
	Super::ExitState(Context, Transition);
}
