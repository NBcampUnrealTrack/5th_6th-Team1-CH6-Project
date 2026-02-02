// Fill out your copyright notice in the Description page of Project Settings.


#include "Building/BaseTurret.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagContainer.h"
#include "Net/UnrealNetwork.h"
#include "Weapon/Abilities/GA_Fire.h"
#include "Weapon/Data/RangedWeaponDataAsset.h"
#include "Components/SphereComponent.h"

ABaseTurret::ABaseTurret()
{
	bReplicates = true;
	PrimaryActorTick.bCanEverTick = true;

	ASC = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("ASC"));
	ASC->SetIsReplicated(true);
	ASC->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);

	TargetSerchingSphere = CreateDefaultSubobject<USphereComponent>(TEXT("TargetSerchingSphere"));
	TargetSerchingSphere->SetupAttachment(RootComponent);

	TargetSerchingSphere->SetSphereRadius(SerchingSphereRadius);
	TargetSerchingSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TargetSerchingSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	TargetSerchingSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
}

void ABaseTurret::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		ASC->InitAbilityActorInfo(this, this);
		GiveDefaultAbilities();

		StartAutoFire();

		TargetSerchingSphere->OnComponentBeginOverlap.AddDynamic(
			this, &ABaseTurret::OnTargetBeginOverlap);

		TargetSerchingSphere->OnComponentEndOverlap.AddDynamic(
			this, &ABaseTurret::OnTargetEndOverlap);

		GetWorldTimerManager().SetTimer(
			TargetSearchTimer,
			this,
			&ABaseTurret::UpdateCurrentTarget,
			TargetSearchInterval,
			true
		);
	}
}

void ABaseTurret::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!HasAuthority())
	{
		return;
	}

	if (!CurrentTarget.IsValid())
	{
		return;
	}

	// ===== 타겟 방향 계산 =====
	const FVector MyLoc = GetActorLocation();
	const FVector TargetLoc = CurrentTarget->GetActorLocation();

	FVector Dir = TargetLoc - MyLoc;
	Dir.Z = 0.f;
	if (Dir.IsNearlyZero())
	{
		return;
	}

	const FRotator DesiredRot = Dir.Rotation();
	const FRotator CurrentRot = StaticMeshComp->GetComponentRotation();

	const FRotator NewRot = FMath::RInterpConstantTo(
		CurrentRot,
		DesiredRot,
		DeltaSeconds,
		TurnSpeedDegPerSec
	);

	StaticMeshComp->SetWorldRotation(NewRot);
}

UAbilitySystemComponent* ABaseTurret::GetAbilitySystemComponent() const
{
	return ASC;
}

FVector ABaseTurret::GetFireStartLocation() const
{
	if (StaticMeshComp && StaticMeshComp->DoesSocketExist(MuzzleSocketName))
	{
		return StaticMeshComp->GetSocketLocation(MuzzleSocketName);
	}

	return GetActorLocation();
}

FVector ABaseTurret::GetFireDirection() const
{
	if (StaticMeshComp && StaticMeshComp->DoesSocketExist(MuzzleSocketName))
	{
		const FTransform SocketTM = StaticMeshComp->GetSocketTransform(MuzzleSocketName, RTS_World);
		return SocketTM.GetUnitAxis(EAxis::X);
	}

	return GetActorForwardVector();
}

UDataAsset* ABaseTurret::GetDataAsset() const
{
	return TurretData;
}

void ABaseTurret::GiveDefaultAbilities()
{
	if (HasAuthority())
	{
		ASC->GiveAbility(FGameplayAbilitySpec(UGA_Fire::StaticClass(), 1));
	}
}

void ABaseTurret::StartAutoFire()
{
	if (HasAuthority() && TurretData)
	{
		const float AttackRate = TurretData->AttackRate;
		GetWorldTimerManager().SetTimer(
			FireTimerHandle,
			this,
			&ABaseTurret::Server_FireTick,
			AttackRate,
			true
		);
	}
	
}

void ABaseTurret::Server_FireTick()
{
	if (HasAuthority())
	{
		const FGameplayTag FireAbilityTag = TurretData->WeaponTag;

		FGameplayTagContainer AbilityTags;
		AbilityTags.AddTag(FireAbilityTag);

		ASC->TryActivateAbilitiesByTag(AbilityTags);
	}
	
}

void ABaseTurret::OnTargetBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor || OtherActor == this)
	{
		return;
	}

	TargetCandidates.AddUnique(OtherActor);
}

void ABaseTurret::OnTargetEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	TargetCandidates.Remove(OtherActor);

	if (CurrentTarget == OtherActor)
	{
		CurrentTarget = nullptr;
	}
}

void ABaseTurret::UpdateCurrentTarget()
{
	AActor* BestTarget = nullptr;
	float BestDistSq = FLT_MAX;

	const FVector MyLoc = GetActorLocation();

	for (int32 i = TargetCandidates.Num() - 1; i >= 0; --i)
	{
		AActor* Candidate = TargetCandidates[i].Get();
		
		if (!IsValid(Candidate))
		{
			TargetCandidates.RemoveAt(i);
			continue;
		}

		const float DistSq = FVector::DistSquared(
			MyLoc,
			Candidate->GetActorLocation()
		);

		if (DistSq < BestDistSq)
		{
			BestDistSq = DistSq;
			BestTarget = Candidate;
		}
	}

	CurrentTarget = BestTarget;
}
