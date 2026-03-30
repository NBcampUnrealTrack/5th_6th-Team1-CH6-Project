// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/StateTree/RotateToTargetTask.h"
#include "Enemy/BaseEnemy/BaseEnemyCharacter.h"
#include "AIController.h"
#include "Components/StateTreeComponent.h"
#include "Enemy/DataAsset/BaseEnemyDataAsset.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Framework/BAGameState.h"
#include "Building/BaseCore.h"
#include "GameFramework/CharacterMovementComponent.h"

URotateToTargetTask::URotateToTargetTask(const FObjectInitializer& ObjectInitializer) : 
	Super(ObjectInitializer)
{
	bShouldCallTick = true;
	bShouldCallTickOnlyOnEvents = true;
}

EStateTreeRunStatus URotateToTargetTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition)
{
	Super::EnterState(Context, Transition);

	if (!IsValid(ContextActor))
	{
		UE_LOG(LogTemp, Error, TEXT("URotateToTargetTask : StartState Error"));
		// 해당 몬스터 제거 후 다시 스폰
		return EStateTreeRunStatus::Failed;
	}

	if (!ensureMsgf(IsValid(ContextActor->BaseEnemyDataAsset), TEXT("URotateToTargetTask : DataAsset Error")))
	{
		return EStateTreeRunStatus::Failed;
	} 
	if (!ensureMsgf(IsValid(ContextActor->BaseEnemyDataAsset->RotateEffect), TEXT("UMoveURotateToTargetTaskToTargetActorTask : RotateEffect Error")))
	{
		return EStateTreeRunStatus::Failed;
	}

	if (!IsValid(TargetActor))
	{
		ContextActor->InitTarget();
		TargetActor = ContextActor->GetTargetActor();
	}
	if (!IsValid(TargetActor))
	{
		return EStateTreeRunStatus::Failed;
	}

	if (UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(ContextActor))
	{
		FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
		ActiveGEHandle = ASC->ApplyGameplayEffectToSelf(ContextActor->BaseEnemyDataAsset->RotateEffect->GetDefaultObject<UGameplayEffect>(), 1.0f, EffectContext);
	}

	CachedAIController = ContextActor->GetController<AAIController>();
	if (!ensureMsgf(CachedAIController.IsValid(), TEXT("URotateToTargetTask : AIController Error")))
	{
		return EStateTreeRunStatus::Failed;
	}
	CachedCharacterMovement = ContextActor->GetCharacterMovement();
	if (CachedCharacterMovement.IsValid())
	{
		CachedCharacterMovement->bOrientRotationToMovement = false;
	}
	CachedAIController->SetFocus(TargetActor);

	bool ShouldTickStart = ShouldRotateStart();
	if (!ShouldTickStart && !CachedCharacterMovement->IsFalling())
	{
		TransitionState();
		return EStateTreeRunStatus::Succeeded;
	}

	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus URotateToTargetTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime)
{
	Super::Tick(Context, DeltaTime);

	if (IsFacingTarget() && !CachedCharacterMovement->IsFalling())
	{
		TransitionState();
		return EStateTreeRunStatus::Succeeded;
	}
	else
	{
		if (!IsValid(TargetActor))
		{
			return EStateTreeRunStatus::Failed;
		}
	}

	return EStateTreeRunStatus::Running;
}

void URotateToTargetTask::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition)
{
	if (CachedAIController.IsValid())
	{
		CachedAIController->ClearFocus(EAIFocusPriority::Gameplay);
		CachedAIController.Reset();
	}
	if (CachedCharacterMovement.IsValid())
	{
		CachedCharacterMovement->bOrientRotationToMovement = true;
		CachedCharacterMovement.Reset();
	}

	// 적용했던 GE_Rotate 제거
	if (ActiveGEHandle.IsValid() && IsValid(ContextActor))
	{
		if (UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(ContextActor))
		{
			ASC->RemoveActiveGameplayEffect(ActiveGEHandle);
		}
		ActiveGEHandle.Invalidate();
	}

	if (IsValid(ContextActor))
	{
		ContextActor->bIsTurning = false;
	}

	Super::ExitState(Context, Transition);
}

bool URotateToTargetTask::ShouldRotateStart()
{
	if (!IsValid(ContextActor) || !IsValid(TargetActor))
	{
		return false;
	}

	FVector ForwardDirection = ContextActor->GetActorForwardVector();
	FVector ToTargetDirection = (TargetActor->GetActorLocation() - ContextActor->GetActorLocation());
	ToTargetDirection.Z = 0.f;	// 정사영
	ToTargetDirection = ToTargetDirection.GetSafeNormal();
	float DotResult = FVector::DotProduct(ForwardDirection, ToTargetDirection);			// 각도 판단
	float CrossResult = FVector::CrossProduct(ToTargetDirection, ForwardDirection).Z;	// 좌우 판단
	float Threshold = FMath::Cos(FMath::DegreesToRadians(RotateThreshold));

	if (DotResult < Threshold)
	{
		ContextActor->bIsTurning = true;
		ContextActor->bIsTurningLeft = (CrossResult > 0);
		return true;
	}
	else
	{
		return false;
	}
}

bool URotateToTargetTask::IsFacingTarget()
{
	if (!IsValid(ContextActor))
	{
		return false;
	}
	if (!IsValid(TargetActor))
	{
		ContextActor->InitTarget();
		TargetActor = ContextActor->GetTargetActor();
		if (!IsValid(TargetActor))
		{
			return false;
		}
	}

	FVector ForwardDirection = ContextActor->GetActorForwardVector();
	FVector ToTargetDirection = (TargetActor->GetActorLocation() - ContextActor->GetActorLocation());
	ToTargetDirection.Z = 0.f;	// 정사영
	ToTargetDirection = ToTargetDirection.GetSafeNormal();
	float DotResult = FVector::DotProduct(ForwardDirection, ToTargetDirection);
	float Threshold = FMath::Cos(FMath::DegreesToRadians(RotateThreshold));

	return DotResult > Threshold ? true : false;
}

void URotateToTargetTask::TransitionState()
{
	if (!IsValid(TargetActor))
	{
		ContextActor->InitTarget();
		TargetActor = ContextActor->GetTargetActor();		
	}
	ContextActor->GetStateTreeComponent()->SendStateTreeEvent(ToMove);
}