// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/StateTreeTaskBlueprintBase.h"
#include "Enemy/BaseEnemy/BaseEnemyCharacter.h"
#include "DieTask.generated.h"

/**
 *
 */
UCLASS()
class BULLETANT_API UDieTask : public UStateTreeTaskBlueprintBase
{
	GENERATED_BODY()

protected:
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	TObjectPtr<ABaseEnemyCharacter> ContextActor;

private:
	FActiveGameplayEffectHandle DeathGEHandle;
};
