// Fill out your copyright notice in the Description page of Project Settings.


#include "Building/BaseTurret.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagContainer.h"
#include "Net/UnrealNetwork.h"
#include "Weapon/Abilities/GA_Fire.h"
#include "Weapon/Data/RangedWeaponDataAsset.h"
#include "Components/SphereComponent.h"
#include "GAS/AttributeSet/HealthAttributeSet.h"
#include "GeometryCollection/GeometryCollectionComponent.h"
#include "GeometryCollection/GeometryCollection.h"

ABaseTurret::ABaseTurret()
{
	bReplicates = true;
	PrimaryActorTick.bCanEverTick = true;

	BodyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BodyMesh"));
	BodyMesh->SetupAttachment(StaticMeshComp);

	BarrelMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BarrelMesh"));
	BarrelMesh->SetupAttachment(BodyMesh);
	BarrelMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	ASC = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("ASC"));
	ASC->SetIsReplicated(true);
	ASC->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);

	HealthSet = CreateDefaultSubobject<UHealthAttributeSet>(TEXT("HealthSet"));

	TargetSerchingSphere = CreateDefaultSubobject<USphereComponent>(TEXT("TargetSerchingSphere"));
	TargetSerchingSphere->SetupAttachment(RootComponent);

	TargetSerchingSphere->SetSphereRadius(SerchingSphereRadius);
	TargetSerchingSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TargetSerchingSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	TargetSerchingSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	DestructionComp = CreateDefaultSubobject<UGeometryCollectionComponent>(TEXT("DestructionComp"));
	DestructionComp->SetupAttachment(RootComponent);
	DestructionComp->SetHiddenInGame(true);
	DestructionComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	DestructionComp->SetSimulatePhysics(false);
	DestructionComp->SetIsReplicated(false);
}

void ABaseTurret::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		ASC->InitAbilityActorInfo(this, this);
		GiveDefaultAbilities();

		StartAutoFire();

		TargetSerchingSphere->OnComponentBeginOverlap.AddDynamic(this, &ABaseTurret::OnTargetBeginOverlap);
		TargetSerchingSphere->OnComponentEndOverlap.AddDynamic(this, &ABaseTurret::OnTargetEndOverlap);

		GetWorldTimerManager().SetTimer(
			TargetSearchTimer,
			this,
			&ABaseTurret::UpdateCurrentTarget,
			TargetSearchInterval,
			true
		);
	}

	if (DestructionCollection)
	{
		DestructionComp->SetRestCollection(DestructionCollection);
	}
}

void ABaseTurret::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bDead)
	{
		return;
	}

	if (IsValid(CurrentTarget))
	{
		const FVector TargetLoc = CurrentTarget->GetActorLocation();

		// ===== 타겟 Yaw 계산 =====
		const FVector BodyLoc = BodyMesh->GetComponentLocation();

		FVector Dir = TargetLoc - BodyLoc;
		Dir.Z = 0.f;

		float DesiredYaw = 0.f;
		if (!Dir.IsNearlyZero())
		{
			DesiredYaw = Dir.Rotation().Yaw;
		}

		const float BodyYaw = BodyMesh->GetComponentRotation().Yaw;
		const float NewYaw = FMath::FixedTurn(BodyYaw, DesiredYaw, TurnSpeedDegPerSec * DeltaSeconds);
		BodyMesh->SetWorldRotation(FRotator(0.f, NewYaw, 0.f));

		// ===== 타겟 Pitch 계산 =====
		const FVector ParrelLoc = BarrelMesh->GetComponentLocation();

		Dir = TargetLoc - ParrelLoc;
		const float Dist = FVector2D(Dir.X, Dir.Y).Size();

		float DesiredPitch = 0.f;
		if (!Dir.IsNearlyZero())
		{
			DesiredPitch = FMath::RadiansToDegrees(FMath::Atan2(Dir.Z, Dist));
		}
		DesiredPitch = FMath::ClampAngle(DesiredPitch, PitchMin, PitchMax);

		const float BarrelPitch = BarrelMesh->GetRelativeRotation().Pitch;
		const float NewPitch = FMath::FixedTurn(BarrelPitch, DesiredPitch, TurnSpeedDegPerSec * DeltaSeconds);
		BarrelMesh->SetRelativeRotation(FRotator(NewPitch, 0.f, 0.f));
	}
}

void ABaseTurret::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABaseTurret, bDead);
	DOREPLIFETIME(ABaseTurret, CurrentTarget);
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

void ABaseTurret::OnDeath()
{
	if (!HasAuthority() || bDead)
	{
		return;
	}

	bDead = true;
	OnRep_Dead();
	Multicast_PlayDestruction(GetActorLocation());
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
		const float AttackRate = TurretData->RoundPerMinute;
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
	if (!HasAuthority())
	{
		return;
	}

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

		const float DistSq = FVector::DistSquared(MyLoc, Candidate->GetActorLocation());

		if (DistSq < BestDistSq)
		{
			BestDistSq = DistSq;
			BestTarget = Candidate;
		}
	}

	CurrentTarget = BestTarget;
}

void ABaseTurret::OnRep_Dead()
{
	if (!bDead)
	{
		return;
	}

	if (StaticMeshComp)
	{
		StaticMeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		StaticMeshComp->SetHiddenInGame(true);
	}
	if (BodyMesh)
	{
		BodyMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		BodyMesh->SetHiddenInGame(true);
	}
	if (BarrelMesh)
	{
		BarrelMesh->SetHiddenInGame(true);
	}
}

void ABaseTurret::Multicast_PlayDestruction_Implementation(const FVector& ImpulseOrigin)
{
	if (!DestructionComp || !DestructionCollection)
	{
		return;
	}

	// 렌더 켬
	DestructionComp->SetHiddenInGame(false);

	// 충돌/물리 켬
	DestructionComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	// 시뮬 켬
	DestructionComp->SetSimulatePhysics(true);

	// 임펄스
	const float Strength = 500.f;
	FVector Dir = (GetActorLocation() - ImpulseOrigin);
	Dir = Dir.IsNearlyZero() ? FVector(1, 0, 1).GetSafeNormal() : Dir.GetSafeNormal();

	DestructionComp->AddImpulse(Dir * Strength, NAME_None, true);
}
