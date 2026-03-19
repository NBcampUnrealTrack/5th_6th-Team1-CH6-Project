// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "Blueprint/StateTreeTaskBlueprintBase.h"
#include "FlyToLoc.generated.h"

class ABaseEnemyCharacter;
class UCharacterMovementComponent;
class UFlyDataAsset;

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

	void FindDestLoc();

	void FlyToProperLoc(const FVector& Current, const FVector& Dest, const float DeltaTime, bool& bCan);

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	TObjectPtr<ABaseEnemyCharacter> ContextEnemy;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	TObjectPtr<AActor> TargetActor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	float AcceptanceRadius = 50.f;

protected:
	TWeakObjectPtr<UCharacterMovementComponent> CMC;

	TObjectPtr<UFlyDataAsset> FlyDataAsset;

	FVector DestLocation;

	FActiveGameplayEffectHandle ActiveGEHandle;
};
