// Fill out your copyright notice in the Description page of Project Settings.

#include "Enemy/StateTree/MoveToTargetActorTask.h"
#include "AIController.h"
#include "GameFramework/Character.h"

EStateTreeRunStatus UMoveToTargetActorTask::EnterState(FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition)
{
	if (!IsValid(ContextActor) || !IsValid(TargetActor))
	{
		return EStateTreeRunStatus::Failed;
	}

	CachedAIController = ContextActor->GetController<AAIController>();
	if (!CachedAIController.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("MoveToTargetActorTask: AIController not found"));
		return EStateTreeRunStatus::Failed;
	}

	// 이동 완료 델리게이트 바인딩
	CachedAIController->ReceiveMoveCompleted.AddDynamic(this, &UMoveToTargetActorTask::OnMoveCompleted);

	StartMoveToTarget();

	if (!bMoveRequestActive)
	{
		// 이미 목표에 있거나 실패
		CachedAIController->ReceiveMoveCompleted.RemoveDynamic(this, &UMoveToTargetActorTask::OnMoveCompleted);
		return EStateTreeRunStatus::Succeeded;
	}

	return EStateTreeRunStatus::Running;
}

void UMoveToTargetActorTask::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition)
{
	if (CachedAIController.IsValid())
	{
		// 델리게이트 해제
		CachedAIController->ReceiveMoveCompleted.RemoveDynamic(this, &UMoveToTargetActorTask::OnMoveCompleted);

		// 이동 중단
		if (bMoveRequestActive)
		{
			CachedAIController->StopMovement();
		}
	}

	bMoveRequestActive = false;
	CachedAIController = nullptr;
}

void UMoveToTargetActorTask::StartMoveToTarget()
{
	if (!CachedAIController.IsValid() || !IsValid(TargetActor))
	{
		bMoveRequestActive = false;
		return;
	}

	EPathFollowingRequestResult::Type Result = CachedAIController->MoveToActor(TargetActor, AcceptanceRadius);

	if (Result == EPathFollowingRequestResult::RequestSuccessful)
	{
		bMoveRequestActive = true;
		CurrentRequestID = CachedAIController->GetCurrentMoveRequestID();
	}
	else if (Result == EPathFollowingRequestResult::AlreadyAtGoal)
	{
		bMoveRequestActive = false;
	}
	else
	{
		// Failed
		bMoveRequestActive = false;
		UE_LOG(LogTemp, Warning, TEXT("MoveToTargetActorTask: MoveToActor failed"));
	}
}

void UMoveToTargetActorTask::OnMoveCompleted(FAIRequestID RequestID, EPathFollowingResult::Type Result)
{
	// 현재 요청이 아니면 무시
	if (RequestID != CurrentRequestID)
	{
		return;
	}

	bMoveRequestActive = false;

	if (Result == EPathFollowingResult::Success)
	{
		// 목표 도착
		if (bContinuousFollow && IsValid(TargetActor))
		{
			// 타겟이 이동했을 수 있으므로 거리 확인 후 다시 추적
			float Distance = FVector::Dist(ContextActor->GetActorLocation(), TargetActor->GetActorLocation());
			if (Distance > AcceptanceRadius)
			{
				StartMoveToTarget();
			}
			// 아직 범위 내면 대기 (다음 프레임에 다시 확인하지 않음 - 성능 최적화)
		}
	}
	else if (Result == EPathFollowingResult::Blocked || Result == EPathFollowingResult::OffPath)
	{
		// 경로 문제 - 잠시 후 재시도
		if (bContinuousFollow && IsValid(TargetActor) && CachedAIController.IsValid())
		{
			// 0.5초 후 재시도
			FTimerHandle RetryTimer;
			GetWorld()->GetTimerManager().SetTimer(RetryTimer, [this]()
			{
				if (IsValid(this) && CachedAIController.IsValid())
				{
					StartMoveToTarget();
				}
			}, 0.5f, false);
		}
	}
	// Aborted는 ExitState에서 StopMovement 호출 시 발생 - 무시
}
