// Fill out your copyright notice in the Description page of Project Settings.

#include "Enemy/StateTree/MoveToTargetActorTask.h"
#include "AIController.h"
#include "GameFramework/Character.h"

EStateTreeRunStatus UMoveToTargetActorTask::EnterState(FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition)
{
	if (IsValid(ContextActor) == false || IsValid(TargetActor) == false)
	{
		return EStateTreeRunStatus::Failed;
	}
	
	ContextActor->GetController<AAIController>()->MoveToActor(TargetActor, AcceptanceRadius);
	
	return EStateTreeRunStatus::Succeeded;
}
