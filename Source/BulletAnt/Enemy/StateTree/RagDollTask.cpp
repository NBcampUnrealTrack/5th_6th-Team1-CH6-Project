// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/StateTree/RagDollTask.h"
#include "AbilitySystemComponent.h"
#include "Enemy/DataAsset/BaseEnemyDataAsset.h"
#include "Enemy/Spawn/SpawnManagerSubsystem.h"
#include "Enemy/BaseEnemy/BaseEnemyCharacter.h"

EStateTreeRunStatus URagDollTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition)
{
	Super::EnterState(Context, Transition);

	if (!IsValid(ContextActor))
	{
		UE_LOG(LogTemp, Error, TEXT("RagDollTask EnterState : ContextActor Error"));
		return EStateTreeRunStatus::Failed;
	}

	ContextActor->Multicast_SetRagDoll();

	UAbilitySystemComponent* ASC = ContextActor->GetAbilitySystemComponent();
	if (!ensureMsgf(ASC, TEXT("DieTask EnterState : ASC Error")))
	{
		return EStateTreeRunStatus::Failed;
	}
	ASC->CancelAllAbilities();

	UBaseEnemyDataAsset* DA = ContextActor->BaseEnemyDataAsset;
	if (!ensureMsgf(DA, TEXT("RagDollTask EnterState : BaseEnemyDataAsset Error")))
	{
		return EStateTreeRunStatus::Failed;
	}
	ContextActor->SetLifeSpan(DA->DeathTime);

	return EStateTreeRunStatus::Running;
}

void URagDollTask::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition)
{
	UWorld* World = GetWorld();
	if (IsValid(World))
	{
		USpawnManagerSubsystem* SpawnManagerSubsystem = GetWorld()->GetSubsystem<USpawnManagerSubsystem>();
		if (IsValid(SpawnManagerSubsystem))
		{
			SpawnManagerSubsystem->OnEnemyDie();
		}
	}

	Super::ExitState(Context, Transition);
}
