// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Spitter/BaseSpitterEnemy.h"
#include "Enemy/DataAsset/SpitterDataAsset.h"
#include "Weapon/Data/MeleeWeaponDataAsset.h"
#include "Enemy/DataAsset/BaseEnemyDataAsset.h"
#include "AbilitySystemComponent.h"
#include "FrameWork/BAGameState.h"
#include "Building/BaseCore.h"

void ABaseSpitterEnemy::StartSpit()
{
	if (!HasAuthority())
	{
		return;
	}

	if (!IsValid(SpitterDataAsset))
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!IsValid(World))
	{
		return;
	}
	World->GetTimerManager().SetTimer(
		DamageChecker,
		this,
		&ABaseSpitterEnemy::CheckContinousSpit,
		SpitterDataAsset->CheckInterval,
		true
	);
}

void ABaseSpitterEnemy::CheckContinousSpit()
{
	if (!GetMesh())
	{
		return;
	}
	if (!IsValid(SpitterDataAsset))
	{
		return;
	}
	if (!IsValid(GetDataAsset()))
	{
		return;
	}

	FVector MouthLocation = GetMesh()->GetSocketLocation(SpitterDataAsset->AttackOrigin);
	FRotator MouthRotator = GetMesh()->GetSocketRotation(SpitterDataAsset->AttackOrigin);
	FVector Forward = MouthRotator.Vector();

	FVector CapsuleCenter = MouthLocation + (Forward * (SpitterDataAsset->PoisonCapsuleHalfHeight));

	FQuat CapsuleRotation = FRotationMatrix::MakeFromZ(Forward).ToQuat();
	FCollisionShape PoisonCapsule = FCollisionShape::MakeCapsule(SpitterDataAsset->PoisonCapsuleRadius, SpitterDataAsset->PoisonCapsuleHalfHeight);

	TArray<FHitResult> OutHits;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	bool bHit = GetWorld()->SweepMultiByChannel(
		OutHits,
		CapsuleCenter, CapsuleCenter,
		CapsuleRotation,
		ECC_GameTraceChannel2,
		PoisonCapsule,
		Params
	);

	DrawDebugCapsule(GetWorld(), CapsuleCenter, SpitterDataAsset->PoisonCapsuleHalfHeight, SpitterDataAsset->PoisonCapsuleRadius, CapsuleRotation, FColor::Green, false, 0.1f);

	if (bHit)
	{
		for (const FHitResult& Hit : OutHits)
		{
			AActor* HitActor = Hit.GetActor();
			if (HitActor)
			{
				FGameplayEventData Payload;
				Payload.Target = HitActor;
				Payload.TargetData = UAbilitySystemBlueprintLibrary::AbilityTargetDataFromActor(HitActor);
				FGameplayTag EventTag = Cast<UMeleeWeaponDataAsset>(GetDataAsset())->HitEventTag;

				AbilitySystemComponent->HandleGameplayEvent(EventTag, &Payload);
			}
		}
	}
}

void ABaseSpitterEnemy::StopSpit()
{
	if (!HasAuthority())
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!IsValid(World))
	{
		return;
	}
	World->GetTimerManager().ClearTimer(DamageChecker);
}

UDataAsset* ABaseSpitterEnemy::GetDataAsset() const
{
	AActor* CurrentTarget = TargetActor;
	if (!IsValid(CurrentTarget))
	{
		UWorld* World = GetWorld();
		if (!IsValid(World))
		{
			UE_LOG(LogTemp, Error, TEXT("ABaseSpitterEnemy GetDataAsset : World Error"));
			return nullptr;
		}

		ABAGameState* GS = GetWorld()->GetGameState<ABAGameState>();
		if (!IsValid(GS))
		{
			UE_LOG(LogTemp, Error, TEXT("ABaseSpitterEnemy GetDataAsset : GameState Error"));
			return nullptr;
		}
		CurrentTarget = GS->GetTargetCore();
	}

	float TargetDistance = FVector::DistSquaredXY(GetActorLocation(), CurrentTarget->GetActorLocation());
	float AssetDistance = 0;
	for (int i = BaseEnemyDataAsset->BaseEnemyAttackDataAssetArray.Num() - 1; i >= 0; i--)
	{
		AssetDistance = BaseEnemyDataAsset->BaseEnemyAttackDataAssetArray[i].Distance;
		AssetDistance *= AssetDistance;
		if (TargetDistance >= AssetDistance)
		{
			return BaseEnemyDataAsset->BaseEnemyAttackDataAssetArray[i].AttackDataAsset;
		}
	}

	return BaseEnemyDataAsset->BaseEnemyAttackDataAssetArray[0].AttackDataAsset;
}
