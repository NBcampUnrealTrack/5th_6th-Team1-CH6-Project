// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/StateTree/AttackTask.h"
#include "Enemy/BaseEnemyCharacter.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "enemy/BaseEnemyDataAsset.h"
#include "GameplayTagContainer.h"
#include "Weapon/Data/WeaponDataAsset.h"

EStateTreeRunStatus UAttackTask::EnterState(FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition)
{
	Super::EnterState(Context, Transition);
	
	if (!IsValid(ContextActor) || !IsValid(TargetActor))
	{
		UE_LOG(LogTemp, Error, TEXT("UAttackTask-EnterState : ContextTargetActor Error"));
		// 해당 몬스터 제거 후 다시 스폰
		return EStateTreeRunStatus::Failed;
	}
	
	checkf(IsValid(ContextActor->BaseEnemyDataAsset), TEXT("UAttackTask-EnterState : DataAsset Error"));
	if (UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(ContextActor))
	{
		UWeaponDataAsset* WeaponDataAsset = Cast<UWeaponDataAsset>(ContextActor->GetDataAsset());
		if (IsValid(WeaponDataAsset))
		{
			ASC->TryActivateAbilitiesByTag(FGameplayTagContainer(WeaponDataAsset->WeaponTag));
		}
	}
	
	return EStateTreeRunStatus::Running;
}

void UAttackTask::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition)
{	
	Super::ExitState(Context, Transition);
}
