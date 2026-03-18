// Fill out your copyright notice in the Description page of Project Settings.


#include "Building/SlowPulseTurret.h"
#include "Enemy/BaseEnemy/BaseEnemyCharacter.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GAS/BAGameplayTags.h"
#include "GameplayEffect.h"
#include "Components/SphereComponent.h"
#include "Engine/World.h"
#include "Engine/OverlapResult.h" 
#include "Building/PulseTurretDataAsset.h"

ASlowPulseTurret::ASlowPulseTurret()
{
	TurretData = PulseTurretData;
}

bool ASlowPulseTurret::CanStartAttack() const
{
	if (!HasAuthority() || bDead)
	{
		return false;
	}

	for (const TWeakObjectPtr<AActor>& Candidate : TargetCandidates)
	{
		if (IsValid(Candidate.Get()))
		{
			return true;
		}
	}

	return false;
}

float ASlowPulseTurret::GetAttackInterval() const
{
	return PulseTurretData->PulseInterval;
}

void ASlowPulseTurret::ExecuteAttack()
{
	if (!HasAuthority() || bDead)
	{
		return;
	}

	TArray<ABaseEnemyCharacter*> HitEnemies;
	GatherPulseTargets(HitEnemies);

	if (HitEnemies.Num() == 0)
	{
		return;
	}

	for (ABaseEnemyCharacter* Enemy : HitEnemies)
	{
		if (!IsValid(Enemy))
		{
			continue;
		}

		ApplyDamageToEnemy(Enemy);
		ApplyEffectToEnemy(Enemy, PulseTurretData->SlowEffectClass);
	}

	Multicast_PlayPulseFX();
}

void ASlowPulseTurret::GatherPulseTargets(TArray<ABaseEnemyCharacter*>& OutEnemies) const
{
	OutEnemies.Reset();

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	TArray<FOverlapResult> Overlaps;

	FCollisionShape SphereShape = FCollisionShape::MakeSphere(PulseTurretData->PulseRadius);
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(SlowPulseTurret), false);
	QueryParams.AddIgnoredActor(this);

	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(PulseTurretData->EnemyTraceChannel);

	const bool bHit = World->OverlapMultiByObjectType(
		Overlaps,
		GetActorLocation(),
		FQuat::Identity,
		ObjectQueryParams,
		SphereShape,
		QueryParams
	);

	if (!bHit)
	{
		return;
	}

	TSet<ABaseEnemyCharacter*> UniqueEnemies;

	for (const FOverlapResult& Result : Overlaps)
	{
		ABaseEnemyCharacter* Enemy = Cast<ABaseEnemyCharacter>(Result.GetActor());
		if (!IsValid(Enemy))
		{
			continue;
		}

		UniqueEnemies.Add(Enemy);
	}

	OutEnemies.Append(UniqueEnemies.Array());
}

void ASlowPulseTurret::ApplyEffectToEnemy(ABaseEnemyCharacter* Enemy, TSubclassOf<UGameplayEffect> EffectClass, float Level) const
{
	if (!Enemy || !EffectClass)
	{
		return;
	}

	UAbilitySystemComponent* SourceASC = GetAbilitySystemComponent();
	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Enemy);

	if (!SourceASC || !TargetASC)
	{
		return;
	}

	FGameplayEffectContextHandle Context = SourceASC->MakeEffectContext();
	Context.AddSourceObject(const_cast<ASlowPulseTurret*>(this));

	FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(EffectClass, Level, Context);
	if (!SpecHandle.IsValid())
	{
		return;
	}

	SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
}

void ASlowPulseTurret::ApplyDamageToEnemy(ABaseEnemyCharacter* Enemy) const
{
	if (!Enemy || !PulseTurretData->DamageEffectClass)
	{
		return;
	}

	UAbilitySystemComponent* SourceASC = GetAbilitySystemComponent();
	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Enemy);

	if (!SourceASC || !TargetASC)
	{
		return;
	}

	FGameplayEffectContextHandle Context = SourceASC->MakeEffectContext();
	Context.AddSourceObject(const_cast<ASlowPulseTurret*>(this));

	FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(PulseTurretData->DamageEffectClass, 1.f, Context);
	if (!SpecHandle.IsValid())
	{
		return;
	}

	SpecHandle.Data->SetSetByCallerMagnitude(TAG_Data_Combat_Damage, PulseTurretData->PulseDamage);

	SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
}

void ASlowPulseTurret::Multicast_PlayPulseFX_Implementation()
{
	// 나중에 Niagara / 사운드 추가
}