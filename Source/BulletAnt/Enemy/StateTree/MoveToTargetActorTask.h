// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/StateTreeTaskBlueprintBase.h"
#include "MoveToTargetActorTask.generated.h"

/**
 * 
 */
UCLASS()
class BULLETANT_API UMoveToTargetActorTask : public UStateTreeTaskBlueprintBase
{
	GENERATED_BODY()
	
protected:
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) override;
	
public:	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	TObjectPtr<ACharacter> ContextActor;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	TObjectPtr<AActor> TargetActor;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	float AcceptanceRadius;
};
