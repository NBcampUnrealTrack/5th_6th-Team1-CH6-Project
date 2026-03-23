// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/StateTreeTaskBlueprintBase.h"
#include "FlyAttackTask.generated.h"

class ABaseEnemyCharacter;
class UFlyDataAsset;

UCLASS()
class BULLETANT_API UFlyAttackTask : public UStateTreeTaskBlueprintBase
{
	GENERATED_BODY()

protected:
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) override;

	void HitCheck();

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	TObjectPtr<ABaseEnemyCharacter> ContextActor;

protected:
	TWeakObjectPtr<UFlyDataAsset> FlyDA;

	UPROPERTY()
	TSet<TWeakObjectPtr<AActor>> HitActors;

	FTimerHandle AttackTimerHandle;
};
