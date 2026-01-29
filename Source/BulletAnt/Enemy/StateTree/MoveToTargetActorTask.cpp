// Fill out your copyright notice in the Description page of Project Settings.

#include "Enemy/StateTree/MoveToTargetActorTask.h"
#include "AIController.h"
#include "Enemy/BaseEnemyCharacter.h"

EStateTreeRunStatus UMoveToTargetActorTask::EnterState(FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition)
{
	Super::EnterState(Context, Transition);
	
	if (!IsValid(ContextActor) || !IsValid(TargetActor))
	{
		UE_LOG(LogTemp, Error, TEXT("UMoveToTargetActorTask : StartState Error"));
		// 해당 몬스터 제거 후 다시 스폰
		return EStateTreeRunStatus::Failed;
	}
	
	CachedAIController = ContextActor->GetController<AAIController>();
	checkf(CachedAIController.IsValid(), TEXT("MoveToTargetActorTask : AIController Error"));
	
	// 적의 이동 완료시 콜백함수 바인딩
	CachedAIController->ReceiveMoveCompleted.AddDynamic(this, &UMoveToTargetActorTask::OnMoveCompleted);
	StartMoveToTarget();
	
	// AI의 이동 요청이 받아들여졌을 경우
	if (MoveRequestResult == EMoveRequestResult::RequestAccepted)
	{
		return EStateTreeRunStatus::Running;
	}
	//  이미 도착한 경우
	else if (MoveRequestResult == EMoveRequestResult::AlreadyArrived)
	{
		CachedAIController->ReceiveMoveCompleted.RemoveDynamic(this, &UMoveToTargetActorTask::OnMoveCompleted);
		return EStateTreeRunStatus::Succeeded;
	}
	else // 요청이 거절당한 경우
	{
		CachedAIController->ReceiveMoveCompleted.RemoveDynamic(this, &UMoveToTargetActorTask::OnMoveCompleted);
		return EStateTreeRunStatus::Failed;
	}
}

void UMoveToTargetActorTask::StartState()
{
	TargetActor = ContextActor->GetTargetActor();
	
	if (!IsValid(ContextActor) || !IsValid(TargetActor))
	{
		UE_LOG(LogTemp, Error, TEXT("%s"), *ContextActor->GetName());
		UE_LOG(LogTemp, Error, TEXT("UMoveToTargetActorTask : StartState Error"));
		// 해당 몬스터 제거 후 다시 스폰
		return;
	}
	
	// ContextActor->OnTargetActor.RemoveAll(this);
	
	CachedAIController = ContextActor->GetController<AAIController>();
	checkf(CachedAIController.IsValid(), TEXT("MoveToTargetActorTask : AIController Error"));
	
	// 적의 이동 완료시 콜백함수 바인딩
	CachedAIController->ReceiveMoveCompleted.AddDynamic(this, &UMoveToTargetActorTask::OnMoveCompleted);
	StartMoveToTarget();
	
	// AI의 이동 요청이 받아들여졌을 경우
	if (MoveRequestResult == EMoveRequestResult::RequestAccepted)
	{
		// return EStateTreeRunStatus::Running;
		return;	
	}
	//  이미 도착한 경우
	else if (MoveRequestResult == EMoveRequestResult::AlreadyArrived)
	{
		CachedAIController->ReceiveMoveCompleted.RemoveDynamic(this, &UMoveToTargetActorTask::OnMoveCompleted);
		// return EStateTreeRunStatus::Succeeded;
		return;	// 공격 State로 전환
	}
	else // 요청이 거절당한 경우
	{
		CachedAIController->ReceiveMoveCompleted.RemoveDynamic(this, &UMoveToTargetActorTask::OnMoveCompleted);
		// return EStateTreeRunStatus::Failed;
		return;	// 몬스터 제거 후 다시 스폰
	}
}

void UMoveToTargetActorTask::ExitState(FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition)
{
	GetWorld()->GetTimerManager().ClearAllTimersForObject(this);
	
	if (CachedAIController.IsValid())
	{
		// 바인딩한거 제거
		CachedAIController->ReceiveMoveCompleted.RemoveDynamic(this, &UMoveToTargetActorTask::OnMoveCompleted);

		// 이 Task에 의해 요청받아 움직이고 있다면 멈춤
		if (MoveRequestResult == EMoveRequestResult::AlreadyArrived)
		{
			CachedAIController->StopMovement();
		}
	}

	MoveRequestResult = EMoveRequestResult::Failed;
	CachedAIController = nullptr;
	
	Super::ExitState(Context, Transition);
}

void UMoveToTargetActorTask::StartMoveToTarget()
{
	if (!CachedAIController.IsValid() || !IsValid(TargetActor))
	{
		MoveRequestResult = EMoveRequestResult::Failed;
		return;
	}

	// 이동을 요청한 결과
	EPathFollowingRequestResult::Type Result = CachedAIController->MoveToActor(TargetActor, AcceptanceRadius);

	if (Result == EPathFollowingRequestResult::RequestSuccessful)
	{
		MoveRequestResult = EMoveRequestResult::RequestAccepted;
		CurrentRequestID = CachedAIController->GetCurrentMoveRequestID();
	}
	else if (Result == EPathFollowingRequestResult::AlreadyAtGoal)
	{
		MoveRequestResult = EMoveRequestResult::AlreadyArrived;
	}
	else	// Failed
	{
		MoveRequestResult = EMoveRequestResult::Failed;
	}
}

// 도착 또는 멈췄을 시의 콜백함수
void UMoveToTargetActorTask::OnMoveCompleted(FAIRequestID RequestID, EPathFollowingResult::Type Result)
{
	// 이 Task가 요청한 움직일 때만 처리
	if (RequestID != CurrentRequestID)
	{
		return;
	}

	MoveRequestResult = EMoveRequestResult::Failed;

	// 목표 도착
	if (Result == EPathFollowingResult::Success)
	{
		if (IsValid(TargetActor))
		{
			// 공격 State로 전환
		}
	}
	// 경로 문제일 경우 (1초 뒤 재시도)
	else if (Result == EPathFollowingResult::Blocked || Result == EPathFollowingResult::OffPath)
	{
		if (IsValid(TargetActor) && CachedAIController.IsValid())
		{
			TWeakObjectPtr<UMoveToTargetActorTask> WeakSelf(this);
			GetWorld()->GetTimerManager().SetTimer(RetryTimer, [WeakSelf]()
			{
				if (WeakSelf.IsValid() && (WeakSelf->CachedAIController).IsValid())
				{
					WeakSelf->StartMoveToTarget();
				}
			}, 1.f, false);
		}
	}
	// Aborted는 ExitState에서 StopMovement 호출 시 발생 - 무시
}


