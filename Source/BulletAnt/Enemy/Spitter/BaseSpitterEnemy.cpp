// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Spitter/BaseSpitterEnemy.h"
#include "Enemy/DataAsset/SpitterDataAsset.h"
#include "Weapon/Data/MeleeWeaponDataAsset.h"
#include "Enemy/DataAsset/BaseEnemyDataAsset.h"
#include "AbilitySystemComponent.h"
#include "FrameWork/BAGameState.h"
#include "Building/BaseCore.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GAS/BAGameplayTags.h"
#include "NiagaraComponent.h"

ABaseSpitterEnemy::ABaseSpitterEnemy()
{
	NiagaraComp = CreateDefaultSubobject<UNiagaraComponent>(TEXT("NiagaraComp"));
	NiagaraComp->SetupAttachment(GetMesh(), FName("Megaspikan_MouthSocket"));
	NiagaraComp->SetRelativeLocation(FVector::ZeroVector);
	NiagaraComp->SetRelativeRotation(FRotator::ZeroRotator);
	NiagaraComp->bAutoActivate = false;
	NiagaraComp->SetCanEverAffectNavigation(false);
}

void ABaseSpitterEnemy::BeginPlay()
{
	Super::BeginPlay();
		
	if (IsValid(NiagaraComp))
	{
		NiagaraComp->SetAsset(SpitterDataAsset->AttackEffect);
	}
}

void ABaseSpitterEnemy::StartSpit()
{
	if (!IsValid(SpitterDataAsset))
	{
		return;
	}

	if (NiagaraComp)
	{
		NiagaraComp->Activate(true);
	}

	if (HasAuthority())
	{
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
	if (!IsValid(AbilitySystemComponent))
	{
		return;
	}

	FVector MouthLocation = GetMesh()->GetSocketLocation(SpitterDataAsset->AttackOrigin);
	FRotator MouthRotator = GetMesh()->GetSocketRotation(SpitterDataAsset->AttackOrigin);
	FVector Forward = MouthRotator.Vector();

	FVector CapsuleCenter = MouthLocation + (Forward * (SpitterDataAsset->PoisonCapsuleHalfHeight));

	FQuat CapsuleRotation = FRotationMatrix::MakeFromZ(Forward).ToQuat();

	if (bIsFirstCheck)
	{
		LastCapsuleLocation = CapsuleCenter;
		LastCapsuleRotation = CapsuleRotation;
		bIsFirstCheck = false;
	}

	FCollisionShape PoisonCapsule = FCollisionShape::MakeCapsule(SpitterDataAsset->PoisonCapsuleRadius, SpitterDataAsset->PoisonCapsuleHalfHeight);

	TArray<FHitResult> OutHits;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	bool bHit = GetWorld()->SweepMultiByChannel(
		OutHits,
		LastCapsuleLocation, CapsuleCenter,
		LastCapsuleRotation,
		ECC_GameTraceChannel2,
		PoisonCapsule,
		Params
	);

	if (bHit)
	{
		UMeleeWeaponDataAsset* WeaponData = Cast<UMeleeWeaponDataAsset>(GetDataAsset());
		if (!IsValid(WeaponData))
		{
			return;
		}
		FGameplayTag EventTag = WeaponData->HitEventTag;

		TSet<AActor*> ProcessedActors;
		ProcessedActors.Reserve(OutHits.Num());
		for (const FHitResult& Hit : OutHits)
		{
			AActor* HitActor = Hit.GetActor();
			if (IsValid(HitActor) && !ProcessedActors.Contains(HitActor))
			{
				ProcessedActors.Add(HitActor);

				UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(HitActor);
				if (IsValid(TargetASC) && IsValid(AbilitySystemComponent))
				{
					FGameplayEffectContextHandle Context = TargetASC->MakeEffectContext();
					Context.AddInstigator(this, this);
					Context.AddHitResult(Hit);

					FGameplayEffectSpecHandle SpecHandle = TargetASC->MakeOutgoingSpec(
						WeaponData->OnUseStateHitEffect,
						1.f,
						Context
					);

					if (SpecHandle.IsValid())
					{
						SpecHandle.Data->SetSetByCallerMagnitude(TAG_Data_Combat_Damage, WeaponData->BaseDamage);
						AbilitySystemComponent->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
					}
				}
			}
		}
	}

	LastCapsuleLocation = CapsuleCenter;
	LastCapsuleRotation = CapsuleRotation;
}

void ABaseSpitterEnemy::StopSpit()
{
	if (NiagaraComp)
	{
		NiagaraComp->DeactivateImmediate();
	}

	if (HasAuthority())
	{
		UWorld* World = GetWorld();
		if (!IsValid(World))
		{
			return;
		}
		World->GetTimerManager().ClearTimer(DamageChecker);
	}	
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
