// Fill out your copyright notice in the Description page of Project Settings.

#include "Enemy/StateTree/MoveToTargetActorTask.h"
#include "Components/StateTreeComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AIController.h"
#include "Enemy/BaseEnemy/BaseEnemyCharacter.h"
#include "Enemy/DataAsset/BaseEnemyDataAsset.h"
#include "Framework/BAGameState.h"
#include "Building/BaseCore.h"

EStateTreeRunStatus UMoveToTargetActorTask::EnterState(FStateTreeExecutionContext& Context,
                                                       const FStateTreeTransitionResult& Transition)
{
	Super::EnterState(Context, Transition);
	
	if (!IsValid(ContextActor))
	{
		UE_LOG(LogTemp, Error, TEXT("UMoveToTargetActorTask : StartState Error"));
		// 해당 몬스터 제거 후 다시 스폰
		return EStateTreeRunStatus::Failed;
	}
	
	if (!ensureMsgf(IsValid(ContextActor->BaseEnemyDataAsset), TEXT("UMoveToTargetActorTask : DataAsset Error")))
	{
		return EStateTreeRunStatus::Failed;
	}
	if (!ensureMsgf(IsValid(ContextActor->BaseEnemyDataAsset->MoveEffect), TEXT("UMoveToTargetActorTask : MoveEffect Error")))
	{
		return EStateTreeRunStatus::Failed;
	}
	if (UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(ContextActor))
	{
		FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
		ActiveGEHandle = ASC->ApplyGameplayEffectToSelf(ContextActor->BaseEnemyDataAsset->MoveEffect->GetDefaultObject<UGameplayEffect>(), 1.0f, EffectContext);
	}
	
	CachedAIController = ContextActor->GetController<AAIController>();
	if (!ensureMsgf(CachedAIController.IsValid(), TEXT("MoveToTargetActorTask : AIController Error")))
	{
		return EStateTreeRunStatus::Failed;
	}
	
	if (!IsValid(TargetActor))
	{
		UWorld* World = GetWorld();
		if (!IsValid(World))
		{
			return EStateTreeRunStatus::Failed;
		}
		ABAGameState* BAGameState = World->GetGameState<ABAGameState>();
		if (!IsValid(BAGameState))
		{
			return EStateTreeRunStatus::Failed;
		}
		TargetActor = BAGameState->GetTargetCore();
	}
	if (!IsValid(TargetActor))
	{
		return EStateTreeRunStatus::Failed;
	}

	// 적의 이동 완료시 콜백함수 바인딩
	CachedAIController->ReceiveMoveCompleted.AddDynamic(this, &UMoveToTargetActorTask::OnMoveCompleted);
	StartMoveToTarget();
	
	if (!CachedAIController.IsValid())
	{
		return EStateTreeRunStatus::Failed;
	}

	// AI의 이동 요청이 받아들여졌을 경우
	if (MoveRequestResult == EMoveRequestResult::RequestAccepted)
	{
		return EStateTreeRunStatus::Running;
	}
	//  이미 도착한 경우
	else if (MoveRequestResult == EMoveRequestResult::AlreadyArrived)
	{
		CachedAIController->ReceiveMoveCompleted.RemoveDynamic(this, &UMoveToTargetActorTask::OnMoveCompleted);
		OnMoveCompleted(CurrentRequestID, EPathFollowingResult::Success);
		return EStateTreeRunStatus::Running;
	}
	else // 요청이 거절당한 경우
	{
		CachedAIController->ReceiveMoveCompleted.RemoveDynamic(this, &UMoveToTargetActorTask::OnMoveCompleted);
		return EStateTreeRunStatus::Failed;
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
		CachedAIController->StopMovement();
	}

	MoveRequestResult = EMoveRequestResult::Failed;
	CachedAIController.Reset();
	
	// 적용했던 GE_Move 제거
	if (ActiveGEHandle.IsValid() && IsValid(ContextActor))
	{
		if (UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(ContextActor))
		{
			ASC->RemoveActiveGameplayEffect(ActiveGEHandle);
		}
		ActiveGEHandle.Invalidate();
	}
	
	Super::ExitState(Context, Transition);
}

void UMoveToTargetActorTask::StartMoveToTarget()
{
	if (!CachedAIController.IsValid())
	{
		MoveRequestResult = EMoveRequestResult::Failed;
		return;
	}
	if (!IsValid(TargetActor))
	{
		UWorld* World = GetWorld();
		if (!IsValid(World))
		{
			MoveRequestResult = EMoveRequestResult::Failed;
			return;
		}
		ABAGameState* BAGameState = World->GetGameState<ABAGameState>();
		if (!IsValid(BAGameState))
		{
			MoveRequestResult = EMoveRequestResult::Failed;
			return;
		}
		TargetActor = BAGameState->GetTargetCore();
	}
	if (!IsValid(TargetActor))
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
		CurrentRequestID = CachedAIController->GetCurrentMoveRequestID();
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
	
	if (!IsValid(ContextActor))
	{
		UE_LOG(LogTemp, Warning, TEXT("UMoveToTargetActorTask-OnMoveCompleted : ContextActor"));
		return;
	}
	if (!IsValid(ContextActor->GetStateTreeComponent()))
	{
		UE_LOG(LogTemp, Warning, TEXT("UMoveToTargetActorTask-OnMoveCompleted : StateTree"));
		return;
	}

	// 목표 도착
	if (Result == EPathFollowingResult::Success)
	{
		// Attack State로 전환
		if (IsValid(TargetActor))
		{
			ContextActor->GetStateTreeComponent()->SendStateTreeEvent(ToAttack);
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
}


