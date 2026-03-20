#include "Weapon/Projectile/MissileProjectile.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "DrawDebugHelpers.h"
#include "GAS/BAGameplayTags.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Sound/SoundBase.h"
#include "Weapon/Data/RangedWeaponDataAsset.h"
#include "Engine/OverlapResult.h" 

AMissileProjectile::AMissileProjectile()
{
	OnActorHit.AddDynamic(this, &AMissileProjectile::HandleActorHit);
}

void AMissileProjectile::BeginPlay()
{
	Super::BeginPlay();
}

void AMissileProjectile::HandleActorHit(
	AActor* SelfActor,
	AActor* OtherActor,
	FVector NormalImpulse,
	const FHitResult& Hit)
{
	if (bExplosionProcessed)
	{
		return;
	}

	if (OtherActor == this)
	{
		return;
	}

	bExplosionProcessed = true;
	Explode(Hit);
}

void AMissileProjectile::Explode(const FHitResult& Hit)
{
	const FVector BaseLocation = Hit.ImpactPoint.IsNearlyZero() ? GetActorLocation() : FVector(Hit.ImpactPoint);
	const FVector ImpactNormal = Hit.ImpactNormal.IsNearlyZero() ? FVector::UpVector : FVector(Hit.ImpactNormal);
	const FVector ExplosionLocation = BaseLocation + ImpactNormal * ExplosionOffsetAlongNormal;
	const FRotator ExplosionRotation = ImpactNormal.Rotation();

	if (ExplosionEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			ExplosionEffect,
			ExplosionLocation,
			ExplosionRotation,
			ExplosionEffectScale,
			true,
			true,
			ENCPoolMethod::AutoRelease,
			true
		);
	}

	if (ExplosionSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			ExplosionSound,
			ExplosionLocation
		);
	}

	if (bDrawExplosionDebug)
	{
		DrawDebugSphere(
			GetWorld(),
			ExplosionLocation,
			ExplosionRadius,
			20,
			FColor::Red,
			false,
			2.0f
		);
	}

	if (HasAuthority())
	{
		ApplyExplosionDamage(ExplosionLocation, Hit.GetActor());
	}
}

void AMissileProjectile::ApplyExplosionDamage(const FVector& ExplosionLocation, AActor* DirectHitActor)
{
	if (ExplosionRadius <= 0.f || ExplosionDamage <= 0.f)
	{
		return;
	}

	if (!CachedOwner || !CachedData || !CachedData->OnUseStateHitEffect)
	{
		return;
	}

	IAbilitySystemInterface* SourceASCInterface = Cast<IAbilitySystemInterface>(CachedOwner);
	if (!SourceASCInterface)
	{
		return;
	}

	UAbilitySystemComponent* SourceASC = SourceASCInterface->GetAbilitySystemComponent();
	if (!SourceASC)
	{
		return;
	}

	TArray<FOverlapResult> OverlapResults;

	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_Pawn);
	ObjectQueryParams.AddObjectTypesToQuery(ECC_GameTraceChannel6);

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(MissileExplosionOverlap), false, this);
	QueryParams.AddIgnoredActor(this);
	QueryParams.AddIgnoredActor(CachedOwner);

	const bool bHasOverlap = GetWorld()->OverlapMultiByObjectType(
		OverlapResults,
		ExplosionLocation,
		FQuat::Identity,
		ObjectQueryParams,
		FCollisionShape::MakeSphere(ExplosionRadius),
		QueryParams
	);

	if (!bHasOverlap)
	{
		return;
	}

	TSet<AActor*> DamagedActors;

	for (const FOverlapResult& Result : OverlapResults)
	{
		AActor* HitActor = Result.GetActor();
		if (!HitActor)
		{
			continue;
		}

		if (DamagedActors.Contains(HitActor))
		{
			continue;
		}

		if (HitActor == this || HitActor == CachedOwner)
		{
			continue;
		}

		IAbilitySystemInterface* TargetASCInterface = Cast<IAbilitySystemInterface>(HitActor);
		if (!TargetASCInterface)
		{
			continue;
		}

		UAbilitySystemComponent* TargetASC = TargetASCInterface->GetAbilitySystemComponent();
		if (!TargetASC)
		{
			continue;
		}

		FHitResult ExplosionHit;
		ExplosionHit.Location = ExplosionLocation;
		ExplosionHit.ImpactPoint = ExplosionLocation;
		ExplosionHit.TraceStart = GetActorLocation();
		ExplosionHit.TraceEnd = ExplosionLocation;

		FGameplayEffectContextHandle Context = SourceASC->MakeEffectContext();
		Context.AddHitResult(ExplosionHit);
		Context.AddInstigator(CachedOwner, this);

		FGameplayEffectSpecHandle Spec =
			SourceASC->MakeOutgoingSpec(CachedData->OnUseStateHitEffect, 1.f, Context);

		if (!Spec.IsValid())
		{
			continue;
		}

		Spec.Data->SetSetByCallerMagnitude(
			TAG_Data_Combat_Damage,
			ExplosionDamage
		);

		SourceASC->ApplyGameplayEffectSpecToTarget(*Spec.Data.Get(), TargetASC);

		DamagedActors.Add(HitActor);
	}
}