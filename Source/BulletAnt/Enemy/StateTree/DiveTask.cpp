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
    MidLocation = TargetActor->GetActorLocation();
    DiveEndLocation = CaculateDiveEndLocation();

    ABaseFlyEnemy* Fly = Cast<ABaseFlyEnemy>(ContextEnemy);
    if (!IsValid(Fly))
    {
        return EStateTreeRunStatus::Failed;
    }
    Fly->SetDiveMode();
    Fly->SetFlySpeed(CMC->MaxFlySpeed * 2);

	return res;
}

EStateTreeRunStatus UDiveTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime)
{
    DiveDuration += DeltaTime;
    if (FMath::IsNearlyZero(DiveTotalTime))
    {
        DiveAlpha = 1.f;
    }
    else
    {
        DiveAlpha = FMath::Clamp(DiveDuration / DiveTotalTime, 0.0f, 1.0f);
    }

    // [핵심] 수평 이동 (Start -> Exit 일직선 선형 보간)
    FVector CurrentHorizontalLoc = FMath::Lerp(StartLocation, DiveEndLocation, DiveAlpha);

    // [핵심] 수직 이동 (Sine 함수 활용)
    // Alpha가 0.0 -> 1.0 갈 때, Sine(0) -> Sine(π) 즉 0 -> 1 -> 0으로 변함
    float Depth = StartLocation.Z - MidLocation.Z;
    float SineAlpha = FMath::Sin(DiveAlpha * PI);

    //// 높이 보간: 시작높이 -> 타겟높이 -> 이탈높이
    //float StartZ = InstanceData.StartLocation.Z;
    //float TargetZ = InstanceData.TargetLocation.Z;
    //float ExitZ = InstanceData.ExitLocation.Z;

    //// 하강 폭 계산 (타겟높이까지 얼마나 내려갈지)
    //float DiveDepth = StartZ - TargetZ;

    //// 최종 Z값: 기본 Lerp로 올라가는 와중에 Sine 값만큼 '아래로' 더해줌 (U자형 완성)
    //float FinalZ = FMath::Lerp(StartZ, ExitZ, Alpha) - (SineAlpha * DiveDepth);
    float FinalZ = StartLocation.Z - SineAlpha * Depth;
    FVector FinalLocation(CurrentHorizontalLoc.X, CurrentHorizontalLoc.Y, FinalZ);

    // --- 공격 수행 타이밍 (Sine 값이 가장 클 때, 즉 최저점 부근) ---
    if (DiveAlpha >= 0.3 && DiveAlpha <= 0.7 && !bAttack) // 예: 90% 이상 내려왔을 때
    {
        // 발사체 발사 소환 로직 추가
        // SpawnProjectile();
        bAttack = true; // 한 번만 공격하도록
    }
    // --------------------------------------------------------

    // CMC에 속도 적용 (위치 기반으로 속도 역산)
    FVector MoveDir = (FinalLocation - ContextEnemy->GetActorLocation()).GetSafeNormal();
    ContextEnemy->AddMovementInput(MoveDir);
    //CMC->Velocity = MoveDir * CMC->MaxFlySpeed; // 공격 시엔 좀 더 빠르게

    // 완료 판정
    if (DiveDuration >= DiveTotalTime)
    {
        return EStateTreeRunStatus::Succeeded; // 태스크 성공 종료 -> 다음 상태(예: Idle)로
    }

    return EStateTreeRunStatus::Running;
}

void UDiveTask::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition)
{
    ABaseFlyEnemy* Fly = Cast<ABaseFlyEnemy>(ContextEnemy);
    Fly->UnSetDiveMode();

    if (CMC.IsValid())
    {
        Fly->SetFlySpeed(CMC->MaxFlySpeed / 2);
    }
	CMC.Reset();

	Super::ExitState(Context, Transition);
}

FVector UDiveTask::CaculateDiveEndLocation()
{
    float Dist = FVector::Dist2D(StartLocation, MidLocation);
    FVector Dir = (MidLocation - StartLocation).GetSafeNormal2D();
    if (Dir.IsNearlyZero())
    {
        Dir = ContextEnemy->GetActorForwardVector();
    }

    FVector Res = StartLocation + Dir * Dist * 2;
    return Res;
}
