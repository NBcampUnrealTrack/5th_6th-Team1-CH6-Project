// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/StateTree/AttackTask.h"
#include "Enemy/BaseEnemyCharacter.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "enemy/BaseEnemyDataAsset.h"

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
	checkf(IsValid(ContextActor->BaseEnemyDataAsset->AttackEffect), TEXT("UMoveToTargetActorTask : AttackEffect Error"));
	if (UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(ContextActor))
	{
		FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
		ActiveGEHandle = ASC->ApplyGameplayEffectToSelf(ContextActor->BaseEnemyDataAsset->AttackEffect->GetDefaultObject<UGameplayEffect>(), 1.0f, EffectContext);
	}
	
	return EStateTreeRunStatus::Succeeded;
}

void UAttackTask::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition)
{
	Super::ExitState(Context, Transition);
}
