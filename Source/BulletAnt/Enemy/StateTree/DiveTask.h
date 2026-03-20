// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/StateTreeTaskBlueprintBase.h"
#include "DiveTask.generated.h"

class ABaseEnemyCharacter;
class UCharacterMovementComponent;

UCLASS()
class BULLETANT_API UDiveTask : public UStateTreeTaskBlueprintBase
{
	GENERATED_BODY()

public:
	UDiveTask(const FObjectInitializer& ObjectInitializer);

protected:
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) override;

	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) override;

	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	TObjectPtr<ABaseEnemyCharacter> ContextEnemy;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	TObjectPtr<AActor> TargetActor;

protected:
	FVector FindAttackPoint();

	TWeakObjectPtr<UCharacterMovementComponent> CMC;

	float DiveTotalTime = 3.f;

	float DiveAlpha = 0.f;
	float DiveDuration = 0.f;

	FVector StartLocation;
	FVector MidLocation;
	FVector DiveEndLocation;

	uint8 bAttack : 1;
};
