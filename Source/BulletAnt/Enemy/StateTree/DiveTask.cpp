// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/StateTree/DiveTask.h"
#include "Enemy/BaseEnemy/BaseEnemyCharacter.h"
#include "Components/StateTreeComponent.h"
#include "Enemy/DataAsset/FlyDataAsset.h"
#include "GameFrameWork/CharacterMovementComponent.h"
#include "Enemy/Fly/BaseFlyEnemy.h"

UDiveTask::UDiveTask(const FObjectInitializer& ObjectInitializer) : 
	Super(ObjectInitializer)
{
	bShouldCallTick = true;
	bShouldCallTickOnlyOnEvents = true;

    bAttack = false;
}

EStateTreeRunStatus UDiveTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition)
{
	EStateTreeRunStatus res = Super::EnterState(Context, Transition);

	if (!IsValid(ContextEnemy))
	{
		UE_LOG(LogTemp, Error, TEXT("UFlyToLoc : ContextEnemy Error"));
		return EStateTreeRunStatus::Failed;
	}
	CMC = ContextEnemy->GetCharacterMovement();
    CMC->bOrientRotationToMovement = true;

	if (!ensureMsgf(IsValid(ContextEnemy->BaseEnemyDataAsset), TEXT("UFlyToLoc : DataAsset Error")))
	{
		return EStateTreeRunStatus::Failed;
	}
	if (!ensureMsgf(IsValid(ContextEnemy->BaseEnemyDataAsset->MoveEffect), TEXT("UFlyToLoc : MoveEffect Error")))
	{
		return EStateTreeRunStatus::Failed;
	}

	UFlyDataAsset* DAFly = Cast<UFlyDataAsset>(ContextEnemy->BaseEnemyDataAsset);
	if (!ensureMsgf(IsValid(DAFly), TEXT("UFlyToLoc : DAFly Error")))
	{
		return EStateTreeRunStatus::Failed;
	}
    DiveTotalTime = DAFly->DiveTotalTime;

    if (!IsValid(TargetActor))
    {
        return EStateTreeRunStatus::Failed;
    }
    StartLocation = ContextEnemy->GetActorLocation();
    MidLocation = FindAttackPoint();

    ABaseFlyEnemy* Fly = Cast<ABaseFlyEnemy>(ContextEnemy);
    if (!IsValid(Fly))
    {
        return EStateTreeRunStatus::Failed;
    }
    Fly->SetDiveMode();
    Fly->SetFlySpeed(CMC->MaxFlySpeed * DAFly->DiveSpeedMultiplier);

	return res;
}

EStateTreeRunStatus UDiveTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime)
{
    DiveDuration += DeltaTime;
    if (DiveDuration >= DiveTotalTime && ContextEnemy->GetActorLocation().Z >= StartLocation.Z)
    {
        return EStateTreeRunStatus::Succeeded;
    }

    if (FMath::IsNearlyZero(DiveTotalTime))
    {
        DiveAlpha = 1.f;
    }
    else
    {
        DiveAlpha = FMath::Clamp(DiveDuration / DiveTotalTime, 0.0f, 1.0f);
    }

    FVector CurrentHorizontalLoc;
    if (DiveAlpha <= 0.5f)
    {
        MidLocation = TargetActor->GetActorLocation();
        CurrentHorizontalLoc = FMath::Lerp(StartLocation, MidLocation, DiveAlpha * 2);
    }
    else
    {
        CurrentHorizontalLoc = FMath::Lerp(MidLocation, MidLocation + (TargetActor->GetActorLocation() - StartLocation), (DiveAlpha - 0.5f) * 2);
    }

    float Depth = FMath::Abs(StartLocation.Z - MidLocation.Z);
    float SineAlpha = FMath::Sin(DiveAlpha * PI);
    float FinalZ = StartLocation.Z - SineAlpha * Depth;
    FVector FinalLocation(CurrentHorizontalLoc.X, CurrentHorizontalLoc.Y, FinalZ);

    if (DiveAlpha >= 0.3 && DiveAlpha <= 0.7 && !bAttack)
    {
        // Attack();
        bAttack = true;
    }

    FVector MoveDir = (FinalLocation - ContextEnemy->GetActorLocation()).GetSafeNormal();
    ContextEnemy->AddMovementInput(MoveDir);

    return EStateTreeRunStatus::Running;
}

void UDiveTask::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition)
{
    ABaseFlyEnemy* Fly = Cast<ABaseFlyEnemy>(ContextEnemy);
    if (IsValid(Fly))
    {
        Fly->UnSetDiveMode();
    }

    UFlyDataAsset* DAFly = Cast<UFlyDataAsset>(ContextEnemy->BaseEnemyDataAsset);
    if (IsValid(DAFly))
    {
        if (CMC.IsValid())
        {
            Fly->SetFlySpeed(CMC->MaxFlySpeed / DAFly->DiveSpeedMultiplier);
            CMC->bOrientRotationToMovement = false;
        }
    }
	CMC.Reset();

	Super::ExitState(Context, Transition);
}

FVector UDiveTask::FindAttackPoint()
{
    FVector Res;
    FHitResult HitResult;

    FVector Start = ContextEnemy->GetActorLocation();
    FVector End = TargetActor->GetActorLocation();

    FCollisionObjectQueryParams ObjectParams;
    ObjectParams.AddObjectTypesToQuery(ECC_GameTraceChannel1);
    ObjectParams.AddObjectTypesToQuery(ECC_GameTraceChannel3);
    ObjectParams.AddObjectTypesToQuery(ECC_GameTraceChannel9);

    bool bHit = GetWorld()->LineTraceSingleByObjectType(
        HitResult,
        Start,
        End,
        ObjectParams
    );

    if (bHit)
    {
        Res = HitResult.ImpactPoint;
    }
    else
    {
        Res = TargetActor->GetActorLocation();
    }
    return Res;
}
