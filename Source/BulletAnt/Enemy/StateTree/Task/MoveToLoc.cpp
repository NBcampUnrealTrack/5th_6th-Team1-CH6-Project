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
#include "Building/BaseBuilding.h"
#include "NavigationSystem.h"
#include "Components/CapsuleComponent.h"

#include "NavigationPath.h"

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

	Target = Cast<ABaseBuilding>(Target);
	if (!IsValid(Target))
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
		Target = BAGameState->GetTargetCore();
	}
	if (!IsValid(Target))
	{
		return EStateTreeRunStatus::Failed;
	}

	if (!CachedAIController.IsValid())
	{
		return EStateTreeRunStatus::Failed;
	}
	// 적의 이동 완료시 콜백함수 바인딩
	CachedAIController->ReceiveMoveCompleted.AddDynamic(this, &UMoveToLoc::OnMoveCompleted);
	StartMoveToTarget();	

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
		ToAttackState();
		return EStateTreeRunStatus::Running;
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
	if (!IsValid(Target))
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
		Target = BAGameState->GetTargetCore();
	}
	if (!IsValid(Target))
	{
		MoveRequestResult = EMoveRequestResult::Failed;
		return;
	}

	FVector ProjectedTargetLocation = GetProjectedTargetLoc();
	// DrawDebugPoint(GetWorld(), ProjectedTargetLocation, 10, FColor::Green, false, 1.f);
	EPathFollowingRequestResult::Type Result;
	Result = CachedAIController->MoveToLocation(ProjectedTargetLocation, AcceptanceRadius);

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

FVector UMoveToLoc::GetProjectedTargetLoc()
{
	std::optional<FVector> CoreLocation;
	FVector TargetLocation;
	if (Target->IsA(ABaseCore::StaticClass()))
	{
		CoreLocation = GetAnchor();
	}
	if (CoreLocation.has_value())
	{
		TargetLocation = *CoreLocation;
	}
	else
	{
		TargetLocation = GetClosestLocation();
	}

	FNavLocation ProjectedLocation;
	bool bCanProject = CanTargetLocProject(TargetLocation, ProjectedLocation);
	if (bCanProject)
	{
		return ProjectedLocation.Location;
	}
	else
	{
		TargetLocation = GetClosestLocation();
		if (CanTargetLocProject(TargetLocation, ProjectedLocation))
		{
			return ProjectedLocation.Location;
		}
		else
		{
			FVector StartLocation = ContextEnemy->GetActorLocation();
			return StartLocation + 0.5 * (TargetLocation - StartLocation);
		}
	}
}

bool UMoveToLoc::CanTargetLocProject(const FVector& Point, FNavLocation& OutLocation)
{
	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	if (IsValid(NavSys))
	{
		float Radius = ContextEnemy->GetCapsuleComponent()->GetScaledCapsuleRadius();
		float HalfHeight = ContextEnemy->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
		const FNavAgentProperties& AgentProps = ContextEnemy->GetNavAgentPropertiesRef();
		return NavSys->ProjectPointToNavigation(Point, OutLocation, FVector(Radius * 1.5f, Radius * 1.5f, HalfHeight * 2.5), &AgentProps);
	}
	return false;
}

const std::optional<FVector> UMoveToLoc::GetAnchor() const
{
	ABaseCore* Core = Cast<ABaseCore>(Target);
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

FVector UMoveToLoc::GetClosestLocation()
{
	FVector StartLocation = ContextEnemy->GetActorLocation();
	FVector ForwardVector = ContextEnemy->GetActorForwardVector();
	float Radius = ContextEnemy->GetCapsuleComponent()->GetScaledCapsuleRadius();
	float HalfHeight = ContextEnemy->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	float TraceDistance = FVector::DistSquared2D(StartLocation, Target->GetActorLocation());
	FVector EndLocation = StartLocation + (ForwardVector * TraceDistance);

	FCollisionShape CollisionShape = FCollisionShape::MakeCapsule(Radius, HalfHeight);
	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_GameTraceChannel1);
	ObjectQueryParams.AddObjectTypesToQuery(ECC_GameTraceChannel9);

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(ContextEnemy);

	FHitResult HitResult;
	bool bHit = GetWorld()->SweepSingleByObjectType(
		HitResult,
		StartLocation,
		EndLocation,
		FQuat::Identity,
		ObjectQueryParams,
		CollisionShape,
		QueryParams
	);

	return bHit ? HitResult.Location : StartLocation + (ForwardVector * TraceDistance);
}

void UMoveToLoc::ToAttackState()
{
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

	if (IsValid(Target))
	{
		ContextEnemy->GetStateTreeComponent()->SendStateTreeEvent(ToAttack);
	}
}

void UMoveToLoc::ToMoveToLocState()
{
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

	if (IsValid(Target))
	{
		FStateTreeEvent ToMove(FGameplayTag::RequestGameplayTag(TEXT("State.Movement.Moving")));
		ContextEnemy->GetStateTreeComponent()->SendStateTreeEvent(ToMove);
	}
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

	// 목표 도착
	if (Result == EPathFollowingResult::Success || Result == EPathFollowingResult::Blocked)
	{
		// Attack State로 전환
		ToAttackState();
	}
	// 경로 문제일 경우 (1초 뒤 재시도)
	else
	{
		if (IsValid(Target) && CachedAIController.IsValid())
		{
			TWeakObjectPtr<UMoveToLoc> WeakSelf(this);
			GetWorld()->GetTimerManager().SetTimer(RetryTimer, [WeakSelf]()
				{
					if (WeakSelf.IsValid() && (WeakSelf->CachedAIController).IsValid())
					{
						WeakSelf->ToMoveToLocState();
					}
				}, 1.f, false);
		}
	}
}


