// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "Blueprint/StateTreeTaskBlueprintBase.h"
#include "Navigation/PathFollowingComponent.h"
#include "Enemy/StateTree/MoveRequestResult.h"
#include "MoveToLoc.generated.h"

class AAIController;
class ABaseEnemyCharacter;

UCLASS()
class BULLETANT_API UMoveToLoc : public UStateTreeTaskBlueprintBase
{
	GENERATED_BODY()

protected:
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) override;

private:
	UFUNCTION()
	void OnMoveCompleted(FAIRequestID RequestID, EPathFollowingResult::Type Result);

	void StartMoveToTarget();

	const std::optional<FVector> GetAnchor() const;
	FVector GetClosestLocation();

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	TObjectPtr<ABaseEnemyCharacter> ContextEnemy;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	TObjectPtr<AActor> Target;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	float AcceptanceRadius = 50.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FStateTreeEvent ToAttack;


protected:
	TWeakObjectPtr<AAIController> CachedAIController;

	FAIRequestID CurrentRequestID;
	EMoveRequestResult MoveRequestResult;

	FActiveGameplayEffectHandle ActiveGEHandle;

	FTimerHandle RetryTimer;
};
