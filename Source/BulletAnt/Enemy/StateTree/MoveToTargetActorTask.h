// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/StateTreeTaskBlueprintBase.h"
#include "Navigation/PathFollowingComponent.h"
#include "MoveToTargetActorTask.generated.h"

class AAIController;

/**
 * StateTree에서 TargetActor를 계속 쫓아가는 Task
 * AcceptanceRadius만큼 가까워지면 멈춤
 * 델리게이트 기반으로 Tick 없이 동작
 */
UCLASS()
class BULLETANT_API UMoveToTargetActorTask : public UStateTreeTaskBlueprintBase
{
	GENERATED_BODY()

protected:
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	TObjectPtr<ACharacter> ContextActor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	TObjectPtr<AActor> TargetActor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	float AcceptanceRadius = 100.f;

	// true면 목표에 도착해도 계속 쫓아감 (타겟이 이동할 경우)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	bool bContinuousFollow = true;

private:
	UFUNCTION()
	void OnMoveCompleted(FAIRequestID RequestID, EPathFollowingResult::Type Result);

	void StartMoveToTarget();

	UPROPERTY()
	TWeakObjectPtr<AAIController> CachedAIController;

	FAIRequestID CurrentRequestID;
	bool bMoveRequestActive = false;
};
