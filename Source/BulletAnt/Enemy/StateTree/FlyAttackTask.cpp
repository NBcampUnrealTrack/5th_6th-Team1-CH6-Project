// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/StateTree/FlyAttackTask.h"
#include "Enemy/BaseEnemy/BaseEnemyCharacter.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Enemy/DataAsset/FlyDataAsset.h"
#include "GameplayTagContainer.h"
#include "Weapon/Data/WeaponDataAsset.h"
#include "GAS/BAGameplayTags.h"

EStateTreeRunStatus UFlyAttackTask::EnterState(FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition)
{
	Super::EnterState(Context, Transition);

	if (!IsValid(ContextActor))
	{
		UE_LOG(LogTemp, Error, TEXT("UAttackTask-EnterState : ContextActor Error"));
		// 해당 몬스터 제거 후 다시 스폰
		return EStateTreeRunStatus::Failed;
	}

	if (!ensureMsgf(IsValid(ContextActor->BaseEnemyDataAsset), TEXT("UAttackTask-EnterState : DataAsset Error")))
	{
		return EStateTreeRunStatus::Failed;
	}
	FlyDA = Cast<UFlyDataAsset>(ContextActor->BaseEnemyDataAsset);
	if (!ensureMsgf(FlyDA.IsValid(), TEXT("UAttackTask-EnterState : FlyDA Error")))
	{
		return EStateTreeRunStatus::Failed;
	}

	UWorld* World = GetWorld();
	if (IsValid(World))
	{
		GetWorld()->GetTimerManager().SetTimer(
			AttackTimerHandle,
			this,
			&UFlyAttackTask::HitCheck,
			0.1f,
			true
		);
	}

	return EStateTreeRunStatus::Running;
}

void UFlyAttackTask::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition)
{
	UWorld* World = GetWorld();
	if (IsValid(World))
	{
		GetWorld()->GetTimerManager().ClearAllTimersForObject(this);
	}

	Super::ExitState(Context, Transition);
}

void UFlyAttackTask::HitCheck()
{
	if (!IsValid(ContextActor))
	{
		return;
	}
	if (!FlyDA.IsValid())
	{
		return;
	}

	FVector BoxOrigin = ContextActor->GetActorLocation();
	FQuat BoxRotator = ContextActor->GetActorQuat();

	TArray<FHitResult> OutHits;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(ContextActor);

	FVector HalfExtent = FVector(FlyDA->AttackBoxHalfDepth, FlyDA->AttackBoxHalfWidth, FlyDA->AttackBoxHalfHeight);
	FCollisionShape BoxShape = FCollisionShape::MakeBox(HalfExtent);
	bool bHit = GetWorld()->SweepMultiByChannel(
		OutHits,
		BoxOrigin,     
		BoxOrigin,      
		BoxRotator,       
		ECC_GameTraceChannel2,
		BoxShape,        
		Params
	);

	DrawDebugBox(GetWorld(), BoxOrigin, HalfExtent, BoxRotator, bHit ? FColor::Red : FColor::Green, false, 0.1f);

	if (bHit)
	{
		UMeleeWeaponDataAsset* WeaponData = Cast<UMeleeWeaponDataAsset>(ContextActor->GetDataAsset());
		if (!IsValid(WeaponData))
		{
			return;
		}
		FGameplayTag EventTag = WeaponData->HitEventTag;

		for (const FHitResult& Hit : OutHits)
		{
			AActor* HitActor = Hit.GetActor();
			if (IsValid(HitActor) && !HitActors.Contains(HitActor))
			{
				HitActors.Add(HitActor);

				UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(HitActor);
				if (IsValid(TargetASC))
				{
					FGameplayEffectContextHandle Context = TargetASC->MakeEffectContext();
					Context.AddInstigator(ContextActor, ContextActor);
					Context.AddHitResult(Hit);

					FGameplayEffectSpecHandle SpecHandle = TargetASC->MakeOutgoingSpec(
						WeaponData->OnUseStateHitEffect,
						1.f,
						Context
					);

					if (SpecHandle.IsValid())
					{
						SpecHandle.Data->SetSetByCallerMagnitude(TAG_Data_Combat_Damage, WeaponData->BaseDamage);
						TargetASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
					}
				}
			}
		}
	}
}
