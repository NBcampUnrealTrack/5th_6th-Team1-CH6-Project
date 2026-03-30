// Fill out your copyright notice in the Description page of Project Settings.


#include "Building/SlowPulseTurret.h"
#include "Enemy/BaseEnemy/BaseEnemyCharacter.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GAS/BAGameplayTags.h"
#include "GameplayEffect.h"
#include "Components/SphereComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"
#include "Engine/OverlapResult.h" 
#include "Building/PulseTurretDataAsset.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

ASlowPulseTurret::ASlowPulseTurret()
{
}

void ASlowPulseTurret::BeginPlay()
{
	TurretData = PulseTurretData;
	Super::BeginPlay();
}

bool ASlowPulseTurret::CanStartAttack() const
{
	return HasAuthority() && !bDead && IsValid(CurrentTarget);
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

		FHitResult HitResult;
		if (FindHitResultOnEnemyCapsule(Enemy, HitResult))
		{
			ApplyDamageToEnemy(Enemy, HitResult);
		}

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
	ObjectQueryParams.AddObjectTypesToQuery(ECC_GameTraceChannel12);

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

void ASlowPulseTurret::ApplyDamageToEnemy(ABaseEnemyCharacter* Enemy, const FHitResult& HitResult) const
{
	if (!Enemy || !PulseTurretData || !PulseTurretData->DamageEffectClass)
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
	Context.AddHitResult(HitResult);
	Context.AddOrigin(HitResult.ImpactPoint);
	Context.AddInstigator(const_cast<ASlowPulseTurret*>(this), const_cast<ASlowPulseTurret*>(this));

	FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(
		PulseTurretData->DamageEffectClass,
		1.f,
		Context
	);

	if (!SpecHandle.IsValid())
	{
		return;
	}

	SpecHandle.Data->SetSetByCallerMagnitude(
		TAG_Data_Combat_Damage,
		PulseTurretData->PulseDamage
	);

	SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
}

bool ASlowPulseTurret::FindHitResultOnEnemyCapsule(ABaseEnemyCharacter* Enemy, FHitResult& OutHitResult) const
{
	OutHitResult = FHitResult();

	if (!Enemy)
	{
		return false;
	}

	UCapsuleComponent* Capsule = Enemy->GetCapsuleComponent();
	if (!Capsule)
	{
		return false;
	}

	const FVector Start = BarrelMesh->GetComponentLocation();
	const FVector End = Capsule->GetComponentLocation();

	FCollisionQueryParams Params(SCENE_QUERY_STAT(PulseCapsuleTrace), false);
	Params.AddIgnoredActor(this);

	// 캡슐 컴포넌트 자체에 직접 라인트레이스
	const bool bHit = Capsule->LineTraceComponent(
		OutHitResult,
		Start,
		End,
		Params
	);

	if (bHit)
	{
		return true;
	}

	return false;
}

void ASlowPulseTurret::Multicast_PlayPulseFX_Implementation()
{
	if (!PulseTurretData)
	{
		return;
	}

	const FVector FXLocation = BarrelMesh->GetComponentLocation();
	const FRotator FXRotation = GetActorRotation();

	if (PulseTurretData->PulseSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			PulseTurretData->PulseSound,
			FXLocation
		);
	}

	if (PulseTurretData->PulseNiagara)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			this,
			PulseTurretData->PulseNiagara,
			FXLocation,
			FXRotation,
			PulseTurretData->FXScale,
			true,
			true
		);
	}
}