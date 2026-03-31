#include "Enemy/StateTree/FlyToLoc.h"
#include "Enemy/BaseEnemy/BaseEnemyCharacter.h"
#include "Components/StateTreeComponent.h"
#include "Enemy/DataAsset/FlyDataAsset.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GameFrameWork/CharacterMovementComponent.h"
#include "AIController.h"

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

	FlyDataAsset = Cast<UFlyDataAsset>(ContextEnemy->BaseEnemyDataAsset);
	if (!ensureMsgf(IsValid(FlyDataAsset), TEXT("UFlyToLoc : DAFly Error")))
	{
		return EStateTreeRunStatus::Failed;
	}

	if (!ensureMsgf(IsValid(TargetActor), TEXT("UFlyToLoc : TargetActor Error")))
	{
		ContextEnemy->InitTarget();
		TargetActor = ContextEnemy->GetTargetActor();
		return EStateTreeRunStatus::Failed;
	}
	FindDestLoc();

	AAIController* AIController = ContextEnemy->GetController<AAIController>();
	if (!ensureMsgf(IsValid(AIController), TEXT("UFlyToLoc : AIController Error")))
	{
		return EStateTreeRunStatus::Failed;
	}
	AIController->SetFocus(TargetActor);

	return res;
}

EStateTreeRunStatus UFlyToLoc::Tick(FStateTreeExecutionContext& Context, const float DeltaTime)
{
	if (!ensureMsgf(IsValid(ContextEnemy), TEXT("UFlyToLoc Tick : ContextEnemy Error")))
	{
		return EStateTreeRunStatus::Failed;
	}
	if (!ensureMsgf(IsValid(TargetActor), TEXT("UFlyToLoc Tick : TargetActor Error")))
	{
		return EStateTreeRunStatus::Failed;
	}
	if (!ensureMsgf(IsValid(FlyDataAsset), TEXT("UFlyToLoc Tick : FlyDataAsset Error")))
	{
		return EStateTreeRunStatus::Failed;
	}
	if (!CMC.IsValid())
	{
		return EStateTreeRunStatus::Failed;
	}

	FVector CurrentLocation = ContextEnemy->GetActorLocation();

	float Dist = FVector::DistSquared2D(CurrentLocation, DestLocation);
	float HorizontalThreshold = FlyDataAsset->DestHorizontalThreshold;
	if (FMath::Abs(Dist) <= HorizontalThreshold * HorizontalThreshold && FMath::Abs(CurrentLocation.Z - DestLocation.Z) <= FlyDataAsset->DestHeightThreshold)
	{
		return EStateTreeRunStatus::Succeeded;
	}

	bool bCan = false;
	FlyToProperLoc(CurrentLocation, DestLocation, DeltaTime, bCan);

	if (!bCan)
	{
		return EStateTreeRunStatus::Failed;
	}

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

void UFlyToLoc::FindDestLoc()
{
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
		DestLocation = HitResult.ImpactPoint;
	}
	else
	{
		DestLocation = TargetActor->GetActorLocation();
	}
	DestLocation.Z += FlyDataAsset->FlyHeight;

	Start.Z = DestLocation.Z;
	FVector DirToStart = (Start - DestLocation).GetSafeNormal();
	DestLocation = DestLocation + (DirToStart * AcceptanceRadius);
}

void UFlyToLoc::FlyToProperLoc(const FVector& Current, const FVector& Dest, const float DeltaTime, bool& bCan)
{
	bCan = false;
	if (!CMC.IsValid())
	{
		return;
	}

	FVector DesiredDir = (Dest - Current).GetSafeNormal();
	ContextEnemy->AddMovementInput(DesiredDir, 1.f);
	bCan = true;
}
