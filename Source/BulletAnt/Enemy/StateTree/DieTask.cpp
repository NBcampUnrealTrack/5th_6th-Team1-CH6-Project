// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/StateTree/DieTask.h"
#include "AIController.h"
#include "AbilitySystemComponent.h"
#include "Enemy/DataAsset/BaseEnemyDataAsset.h"
#include "Enemy/Spawn/SpawnManagerSubsystem.h"

EStateTreeRunStatus UDieTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition)
{
	Super::EnterState(Context, Transition);

	if (!IsValid(ContextActor))
	{
		UE_LOG(LogTemp, Error, TEXT("UMoveToTargetActorTask : StartState Error"));
		return EStateTreeRunStatus::Failed;
	}

	if (AAIController* AIController = ContextActor->GetController<AAIController>())
	{
		AIController->StopMovement();
	}

	ContextActor->Multicast_SetNoCollision();

	//AbilitySystemComponent->CancelAllAbilities();
	UAbilitySystemComponent* ASC = ContextActor->GetAbilitySystemComponent();
	if (!ensureMsgf(ASC, TEXT("DieTask EnterState : ASC Error")))
	{
		return EStateTreeRunStatus::Failed;
	}
	UBaseEnemyDataAsset* DA = ContextActor->BaseEnemyDataAsset;
	if (!ensureMsgf(DA, TEXT("DieTask EnterState : BaseEnemyDataAsset Error")))
	{
		return EStateTreeRunStatus::Failed;
	}

	FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
	if (IsValid(DA->DeathEffect))
	{
		DeathGEHandle = ASC->ApplyGameplayEffectToSelf(DA->DeathEffect->GetDefaultObject<UGameplayEffect>(), 1.0f, EffectContext);
	}

	return EStateTreeRunStatus::Running;
}

void UDieTask::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition)
{
	UAbilitySystemComponent* ASC = ContextActor->GetAbilitySystemComponent();
	if (!ensureMsgf(ASC, TEXT("DieTask EnterState : ASC Error")))
	{
		Super::ExitState(Context, Transition);
		return;
	}

	if (ASC)
	{
		if (DeathGEHandle.IsValid())
		{
			ASC->RemoveActiveGameplayEffect(DeathGEHandle);
			DeathGEHandle.Invalidate();
		}

		ASC->CancelAllAbilities();
	}

	ContextActor->Destroy();

	UWorld* World = GetWorld();
	if (IsValid(World))
	{
		USpawnManagerSubsystem* SpawnManagerSubsystem = GetWorld()->GetSubsystem<USpawnManagerSubsystem>();
		if (IsValid(SpawnManagerSubsystem))
		{
			SpawnManagerSubsystem->OnEnemyDie();
		}
	}

	Super::ExitState(Context, Transition);
}
