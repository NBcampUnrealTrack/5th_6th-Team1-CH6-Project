// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/StateTreeTaskBlueprintBase.h"
#include "Navigation/PathFollowingComponent.h"
#include "MoveToTargetActorTask.generated.h"

class AAIController;
class ABaseEnemyCharacter;

enum class EMoveRequestResult : uint8
{
	RequestAccepted,
	AlreadyArrived,
	Failed            
};

UCLASS()
class BULLETANT_API UMoveToTargetActorTask : public UStateTreeTaskBlueprintBase
{
	GENERATED_BODY()
	
protected:
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) override;
	
private:
	UFUNCTION()
	void StartState();	
	
	UFUNCTION()
	void OnMoveCompleted(FAIRequestID RequestID, EPathFollowingResult::Type Result);
	
	void StartMoveToTarget();
	
public:	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	TObjectPtr<ABaseEnemyCharacter> ContextActor;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	TObjectPtr<AActor> TargetActor;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	float AcceptanceRadius;
	
protected:
	TWeakObjectPtr<AAIController> CachedAIController;

	FAIRequestID CurrentRequestID;
	EMoveRequestResult MoveRequestResult;
	
	FTimerHandle RetryTimer;
};
