// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "Blueprint/StateTreeTaskBlueprintBase.h"
#include "FlyToLoc.generated.h"

class ABaseEnemyCharacter;
class UCharacterMovementComponent;

UCLASS()
class BULLETANT_API UFlyToLoc : public UStateTreeTaskBlueprintBase
{
	GENERATED_BODY()
	
public:
	UFlyToLoc(const FObjectInitializer& ObjectInitializer);

protected:
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) override;

	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) override;

	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	TObjectPtr<ABaseEnemyCharacter> ContextEnemy;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	TObjectPtr<AActor> TargetActor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	float AcceptanceRadius = 50.f;

protected:
	TWeakObjectPtr<UCharacterMovementComponent> CMC;

	float TurnSpeed = 3.0f;

	float AccelerationRate = 2.0f;

	FActiveGameplayEffectHandle ActiveGEHandle;
};
