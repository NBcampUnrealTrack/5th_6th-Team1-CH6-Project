#include "Enemy/StateTree/FlyToLoc.h"
#include "Enemy/BaseEnemy/BaseEnemyCharacter.h"
#include "Components/StateTreeComponent.h"
#include "Enemy/DataAsset/FlyDataAsset.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GameFrameWork/CharacterMovementComponent.h"

UFlyToLoc::UFlyToLoc(const FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer)
{
	bShouldCallTick = true;
	bShouldCallTickOnlyOnEvents = true;
}

EStateTreeRunStatus UFlyToLoc::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition)
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
	if (UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(ContextEnemy))
	{
		FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
		ActiveGEHandle = ASC->ApplyGameplayEffectToSelf(ContextEnemy->BaseEnemyDataAsset->MoveEffect->GetDefaultObject<UGameplayEffect>(), 1.0f, EffectContext);
	}

	UFlyDataAsset* DAFly = Cast<UFlyDataAsset>(ContextEnemy->BaseEnemyDataAsset);
	if (!ensureMsgf(IsValid(DAFly), TEXT("UFlyToLoc : DAFly Error")))
	{
		return EStateTreeRunStatus::Failed;
	}
	TurnSpeed = DAFly->TurnSpeed;
	AccelerationRate = DAFly->AccelerationRate;

	return res;
}

EStateTreeRunStatus UFlyToLoc::Tick(FStateTreeExecutionContext& Context, const float DeltaTime)
{
	// 1. 필요한 데이터 가져오기
	if (!ensureMsgf(IsValid(ContextEnemy), TEXT("UFlyToLoc Tick : ContextEnemy Error")))
	{
		return EStateTreeRunStatus::Failed;
	}
	if (!ensureMsgf(IsValid(TargetActor), TEXT("UFlyToLoc Tick : TargetActor Error")))
	{
		return EStateTreeRunStatus::Failed;
	}
	FVector CurrentLocation = ContextEnemy->GetActorLocation();
	FVector TargetLocation = TargetActor->GetActorLocation();
	TargetLocation.Z = CurrentLocation.Z;

	float Dist = FVector::DistSquared2D(CurrentLocation, TargetLocation);
	if (AcceptanceRadius * AcceptanceRadius >= Dist)
	{
		return EStateTreeRunStatus::Succeeded;
	}

	// 2. 방향 계산
	if (!CMC.IsValid())
	{
		return EStateTreeRunStatus::Failed;
	}
	FVector CurrentVel = CMC->Velocity;
	FVector DesiredDir = (TargetLocation - CurrentLocation).GetSafeNormal();

	// 3. 속도 크기(Speed) 결정
	// 현재 속도가 너무 느리면 최소 속도로 시작, 아니면 MaxFlySpeed까지 가속
	float CurrentSpeed = CurrentVel.Size();
	if (CurrentSpeed < 10.f) CurrentSpeed = 100.f;
	float TargetSpeed = CMC->MaxFlySpeed;
	float NewSpeed = FMath::FInterpTo(CurrentSpeed, TargetSpeed, DeltaTime, AccelerationRate);

	// 4. 핵심: 방향 보간 (VInterpTo)
	// 현재 속도의 '방향'을 타겟 방향으로 TurnSpeed만큼 서서히 꺾음
	FVector CurrentDir = CurrentVel.GetSafeNormal();
	if (CurrentDir.IsNearlyZero()) CurrentDir = ContextEnemy->GetActorForwardVector();

	FVector NewDir = FMath::VInterpTo(CurrentDir, DesiredDir, DeltaTime, TurnSpeed);

	// 5. 최종 Velocity 적용 및 동기화
	// CMC의 Velocity를 직접 수정하면 CMC가 다음 프레임에 위치를 계산하고 복제(Replicate)함
	CMC->Velocity = NewDir.GetSafeNormal() * NewSpeed;

	return EStateTreeRunStatus::Running;
}

void UFlyToLoc::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition)
{
	CMC.Reset();

	if (ActiveGEHandle.IsValid() && IsValid(ContextEnemy))
	{
		if (UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(ContextEnemy))
		{
			ASC->RemoveActiveGameplayEffect(ActiveGEHandle);
		}
		ActiveGEHandle.Invalidate();
	}

	Super::ExitState(Context, Transition);
}
