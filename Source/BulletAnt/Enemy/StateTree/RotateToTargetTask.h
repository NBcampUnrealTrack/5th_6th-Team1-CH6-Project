// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "Blueprint/StateTreeTaskBlueprintBase.h"
#include "RotateToTargetTask.generated.h"

class ABaseEnemyCharacter;
class AAIController;

UCLASS()
class BULLETANT_API URotateToTargetTask : public UStateTreeTaskBlueprintBase
{
	GENERATED_BODY()

public:
	URotateToTargetTask(const FObjectInitializer& ObjectInitializer);
	
protected:
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) override;

	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) override;

	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) override;

	bool ShouldRotateStart();

	bool IsFacingTarget();

	void TransitionState();

	AActor* GetCore();

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	TObjectPtr<ABaseEnemyCharacter> ContextActor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	TObjectPtr<AActor> TargetActor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	float RotateThreshold;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FStateTreeEvent ToMove;

protected:
	TWeakObjectPtr<AAIController> CachedAIController;

	FActiveGameplayEffectHandle ActiveGEHandle;
};
