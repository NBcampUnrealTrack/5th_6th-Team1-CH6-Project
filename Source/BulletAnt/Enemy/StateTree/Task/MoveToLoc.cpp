// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/StateTree/Task/MoveToLoc.h"
#include "Components/StateTreeComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AIController.h"
#include "Enemy/BaseEnemy/BaseEnemyCharacter.h"
#include "Enemy/DataAsset/BaseEnemyDataAsset.h"
#include "Framework/BAGameState.h"
#include "Building/BaseCore.h"
#include <optional>
#include "NavigationSystem.h"
#include "Components/CapsuleComponent.h"

EStateTreeRunStatus UMoveToLoc::EnterState(FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition)
{
	Super::EnterState(Context, Transition);

	if (!IsValid(ContextEnemy))
	{
		UE_LOG(LogTemp, Error, TEXT("UMoveToLoc : StartState Error"));
		// 해당 몬스터 제거 후 다시 스폰
		return EStateTreeRunStatus::Failed;
	}

	if (!ensureMsgf(IsValid(ContextEnemy->BaseEnemyDataAsset), TEXT("UMoveToLoc : DataAsset Error")))
	{
		return EStateTreeRunStatus::Failed;
	}
	if (!ensureMsgf(IsValid(ContextEnemy->BaseEnemyDataAsset->MoveEffect), TEXT("UMoveToLoc : MoveEffect Error")))
	{
		return EStateTreeRunStatus::Failed;
	}
	if (UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(ContextEnemy))
	{
		FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
		ActiveGEHandle = ASC->ApplyGameplayEffectToSelf(ContextEnemy->BaseEnemyDataAsset->MoveEffect->GetDefaultObject<UGameplayEffect>(), 1.0f, EffectContext);
	}

	CachedAIController = ContextEnemy->GetController<AAIController>();
	if (!ensureMsgf(CachedAIController.IsValid(), TEXT("MoveToLocationTask : AIController Error")))
	{
		return EStateTreeRunStatus::Failed;
	}

	if (!IsValid(TargetCore))
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
		TargetCore = BAGameState->GetTargetCore();
	}
	if (!IsValid(TargetCore))
	{
		return EStateTreeRunStatus::Failed;
	}

	// 적의 이동 완료시 콜백함수 바인딩
	CachedAIController->ReceiveMoveCompleted.AddDynamic(this, &UMoveToLoc::OnMoveCompleted);
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
		CachedAIController->ReceiveMoveCompleted.RemoveDynamic(this, &UMoveToLoc::OnMoveCompleted);
		OnMoveCompleted(CurrentRequestID, EPathFollowingResult::Success);
		return EStateTreeRunStatus::Running;
	}
	else // 요청이 거절당한 경우
	{
		CachedAIController->ReceiveMoveCompleted.RemoveDynamic(this, &UMoveToLoc::OnMoveCompleted);
		return EStateTreeRunStatus::Failed;
	}
}

void UMoveToLoc::ExitState(FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition)
{
	GetWorld()->GetTimerManager().ClearAllTimersForObject(this);

	if (CachedAIController.IsValid())
	{
		// 바인딩한거 제거
		CachedAIController->ReceiveMoveCompleted.RemoveDynamic(this, &UMoveToLoc::OnMoveCompleted);
		CachedAIController->StopMovement();
	}

	MoveRequestResult = EMoveRequestResult::Failed;
	CachedAIController.Reset();

	// 적용했던 GE_Move 제거
	if (ActiveGEHandle.IsValid() && IsValid(ContextEnemy))
	{
		if (UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(ContextEnemy))
		{
			ASC->RemoveActiveGameplayEffect(ActiveGEHandle);
		}
		ActiveGEHandle.Invalidate();
	}

	Super::ExitState(Context, Transition);
}

void UMoveToLoc::StartMoveToTarget()
{
	if (!CachedAIController.IsValid())
	{
		MoveRequestResult = EMoveRequestResult::Failed;
		return;
	}
	if (!IsValid(TargetCore))
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
		TargetCore = BAGameState->GetTargetCore();
	}
	if (!IsValid(TargetCore))
	{
		MoveRequestResult = EMoveRequestResult::Failed;
		return;
	}

	// 이동을 요청한 결과
	auto TargetLocation = GetAnchor();
	EPathFollowingRequestResult::Type Result;
	if (TargetLocation.has_value())
	{
		FNavLocation ProjectedLocation;
		UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
		if (IsValid(NavSys))
		{
			float Radius = ContextEnemy->GetCapsuleComponent()->GetScaledCapsuleRadius();
			float HalfHeight = ContextEnemy->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
			const FNavAgentProperties& AgentProps = ContextEnemy->GetNavAgentPropertiesRef();
			bool bCanProject = NavSys->ProjectPointToNavigation(*TargetLocation, ProjectedLocation, FVector(Radius, Radius, 10), &AgentProps);
			if (bCanProject)
			{
				Result = CachedAIController->MoveToLocation(ProjectedLocation, AcceptanceRadius);
				DrawDebugPoint(GetWorld(), ProjectedLocation, 10, FColor::Red, true);
			}
			else
			{
				Result = CachedAIController->MoveToLocation(*TargetLocation, AcceptanceRadius);
			}
		}
	}
	else
	{
		MoveRequestResult = EMoveRequestResult::Failed;
		return;
	}
	

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

const std::optional<FVector> UMoveToLoc::GetAnchor() const
{
	ABaseCore* Core = Cast<ABaseCore>(TargetCore);
	if (!IsValid(Core))
	{
		UE_LOG(LogTemp, Error, TEXT("UMoveToLoc GetAnchor : Anchor Miss"));
		return std::nullopt;
	}

	FVector Direction = (ContextEnemy->GetActorLocation() - Core->GetActorLocation()).GetSafeNormal2D();
	float Radians = FMath::Atan2(Direction.Y, Direction.X);
	float Degrees = FMath::RadiansToDegrees(Radians);
	if (Degrees < 0)
	{
		Degrees += 360.f;
	}

	const TArray<FVector>& Anchors = Core->GetAnchors();
	if (!ensureMsgf(Anchors.Num() != 0, TEXT("UMoveToLoc GetAnchor : Anchors Num is Zero")))
	{
		return std::nullopt;
	}
	int32 AnchorIndex = FMath::RoundToInt(Degrees / (360.f / Anchors.Num())) % Anchors.Num();

	return Anchors[AnchorIndex];
}

// 도착 또는 멈췄을 시의 콜백함수
void UMoveToLoc::OnMoveCompleted(FAIRequestID RequestID, EPathFollowingResult::Type Result)
{
	// 이 Task가 요청한 움직일 때만 처리
	if (RequestID != CurrentRequestID)
	{
		return;
	}

	MoveRequestResult = EMoveRequestResult::Failed;

	if (!IsValid(ContextEnemy))
	{
		UE_LOG(LogTemp, Warning, TEXT("UMoveToLoc-OnMoveCompleted : ContextActor"));
		return;
	}
	if (!IsValid(ContextEnemy->GetStateTreeComponent()))
	{
		UE_LOG(LogTemp, Warning, TEXT("UMoveToLoc-OnMoveCompleted : StateTree"));
		return;
	}

	// 목표 도착
	if (Result == EPathFollowingResult::Success)
	{
		// Attack State로 전환
		if (IsValid(TargetCore))
		{
			ContextEnemy->GetStateTreeComponent()->SendStateTreeEvent(ToAttack);
		}
	}
	// 경로 문제일 경우 (1초 뒤 재시도)
	else if (Result == EPathFollowingResult::Blocked || Result == EPathFollowingResult::OffPath)
	{
		if (IsValid(TargetCore) && CachedAIController.IsValid())
		{
			TWeakObjectPtr<UMoveToLoc> WeakSelf(this);
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


