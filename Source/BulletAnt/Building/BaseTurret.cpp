// Fill out your copyright notice in the Description page of Project Settings.


#include "Building/BaseTurret.h"
#include "Components/SphereComponent.h"
#include "Net/UnrealNetwork.h"
#include "Building/TurretDataAsset.h"
#include "Enemy/BaseEnemy/BaseEnemyCharacter.h"
#include "AbilitySystemComponent.h"
#include "Engine/OverlapResult.h"

ABaseTurret::ABaseTurret()
{
	bReplicates = true;
	PrimaryActorTick.bCanEverTick = true;
	SetActorTickEnabled(false);

	BodyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BodyMesh"));
	BodyMesh->SetupAttachment(StaticMeshComp);
	BodyMesh->SetCanEverAffectNavigation(false);

	BarrelMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BarrelMesh"));
	BarrelMesh->SetupAttachment(BodyMesh);
	BarrelMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BarrelMesh->SetCanEverAffectNavigation(false);
}

void ABaseTurret::BeginPlay()
{
	Super::BeginPlay();

	ApplyTurretData();

	if (IsPreviewMode())
	{
		return;
	}

	if (HasAuthority() && TurretData)
	{
		const float InitialDelay = FMath::FRandRange(0.f, TurretData->TargetSearchInterval);

		GetWorldTimerManager().SetTimer(
			TargetSearchTimer,
			this,
			&ThisClass::UpdateCurrentTarget,
			TurretData->TargetSearchInterval,
			true,
			InitialDelay
		);
	}
}

void ABaseTurret::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bDead)
	{
		return;
	}

	UpdateAim(DeltaSeconds);
}

void ABaseTurret::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABaseTurret, CurrentTarget);
}

void ABaseTurret::ApplyPreviewMode()
{
	Super::ApplyPreviewMode();

	BodyMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BarrelMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	const int32 BodyMaterials = BodyMesh->GetNumMaterials();
	for (int32 i = 0; i < BodyMaterials; ++i)
	{
		BodyMesh->SetMaterial(i, PreviewMID);
	}

	const int32 BarrelMaterials = BarrelMesh->GetNumMaterials();
	for (int32 i = 0; i < BarrelMaterials; ++i)
	{
		BarrelMesh->SetMaterial(i, PreviewMID);
	}

	if(ASC)
	{
		ASC->SetComponentTickEnabled(false);
	}
	
	CurrentTarget = nullptr;
}

void ABaseTurret::OnDeath()
{
	Super::OnDeath();
}

void ABaseTurret::OnRep_Dead()
{
	Super::OnRep_Dead();

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

bool ABaseTurret::CanStartAttack() const
{
	return HasAuthority() && !bDead && IsValid(CurrentTarget);
}

float ABaseTurret::GetAttackInterval() const
{
	return 0.f;
}

void ABaseTurret::ExecuteAttack()
{
}

void ABaseTurret::ApplyTurretData()
{
}

void ABaseTurret::UpdateAim(float DeltaSeconds)
{
	if (!TurretData || !IsValid(CurrentTarget))
	{
		return;
	}

	const FVector TargetLoc = CurrentTarget->GetActorLocation();

	if (BodyMesh)
	{
		const FVector BodyLoc = BodyMesh->GetComponentLocation();

		FVector Dir = TargetLoc - BodyLoc;
		Dir.Z = 0.f;

		if (!Dir.IsNearlyZero())
		{
			const float DesiredYaw = Dir.Rotation().Yaw;
			const float BodyYaw = BodyMesh->GetComponentRotation().Yaw;
			const float NewYaw = FMath::FixedTurn(BodyYaw, DesiredYaw, TurretData->TurnSpeedDegPerSec * DeltaSeconds);
			BodyMesh->SetWorldRotation(FRotator(0.f, NewYaw, 0.f));
		}
	}

	if (BarrelMesh)
	{
		const FVector BarrelLoc = BarrelMesh->GetComponentLocation();
		FVector Dir = TargetLoc - BarrelLoc;

		if (!Dir.IsNearlyZero())
		{
			const float Dist = FVector2D(Dir.X, Dir.Y).Size();
			float DesiredPitch = FMath::RadiansToDegrees(FMath::Atan2(Dir.Z, Dist));
			DesiredPitch = FMath::ClampAngle(DesiredPitch, TurretData->PitchMin, TurretData->PitchMax);

			const float BarrelPitch = BarrelMesh->GetRelativeRotation().Pitch;
			const float NewPitch = FMath::FixedTurn(BarrelPitch, DesiredPitch, TurretData->TurnSpeedDegPerSec * DeltaSeconds);
			BarrelMesh->SetRelativeRotation(FRotator(NewPitch, 0.f, 0.f));
		}
	}
}

AActor* ABaseTurret::SelectBestTarget() const
{
	UWorld* World = GetWorld();
	if (!World || !TurretData)
	{
		return nullptr;
	}

	TArray<FOverlapResult> Overlaps;

	FCollisionObjectQueryParams ObjectQuery;
	ObjectQuery.AddObjectTypesToQuery(TurretData->EnemyTraceChannel);
	ObjectQuery.AddObjectTypesToQuery(ECC_GameTraceChannel12); // Fly

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(TurretTargetSearch), false);
	QueryParams.AddIgnoredActor(this);

	const FCollisionShape SphereShape = FCollisionShape::MakeSphere(TurretData->SearchRadius);

	const bool bHit = World->OverlapMultiByObjectType(
		Overlaps,
		GetActorLocation(),
		FQuat::Identity,
		ObjectQuery,
		SphereShape,
		QueryParams
	);

	if (!bHit)
	{
		return nullptr;
	}

	AActor* BestTarget = nullptr;
	float BestDistSq = FLT_MAX;
	const FVector MyLoc = GetActorLocation();

	for (const FOverlapResult& OverResult : Overlaps)
	{
		AActor* Candidate = OverResult.GetActor();
		if (!IsValid(Candidate))
		{
			continue;
		}

		const float DistSq = FVector::DistSquared(MyLoc, Candidate->GetActorLocation());
		if (DistSq < BestDistSq)
		{
			BestDistSq = DistSq;
			BestTarget = Candidate;
		}
	}

	return BestTarget;
}

void ABaseTurret::StartFireLoop()
{
	if (!HasAuthority() || !ASC || !TurretData)
	{
		return;
	}

	if (FireLoopTimerHandle.IsValid())
	{
		return;
	}

	const float AttackInterval = GetAttackInterval();
	if (AttackInterval <= 0.f)
	{
		return;
	}

	GetWorldTimerManager().SetTimer(
		FireLoopTimerHandle,
		this,
		&ABaseTurret::HandleAttackTick,
		AttackInterval,
		true,
		0.f
	);
}

void ABaseTurret::StopFireLoop()
{
	if (FireLoopTimerHandle.IsValid())
	{
		GetWorldTimerManager().ClearTimer(FireLoopTimerHandle);
		FireLoopTimerHandle.Invalidate();
	}
}

void ABaseTurret::HandleAttackTick()
{
	if (!CanStartAttack())
	{
		return;
	}

	ExecuteAttack();
}

void ABaseTurret::UpdateCurrentTarget()
{
	if (!HasAuthority() || bDead || !TurretData)
	{
		return;
	}

	AActor* PrevTarget = CurrentTarget;

	if (IsValid(CurrentTarget))
	{
		const float DistSq = FVector::DistSquared(GetActorLocation(), CurrentTarget->GetActorLocation());
		if (DistSq <= FMath::Square(TurretData->SearchRadius))
		{
			return;
		}

		CurrentTarget = nullptr;
	}

	CurrentTarget = SelectBestTarget();

	if (!IsValid(PrevTarget) && IsValid(CurrentTarget))
	{
		SetActorTickEnabled(true);
		StartFireLoop();
	}
	else if (IsValid(PrevTarget) && !IsValid(CurrentTarget))
	{
		SetActorTickEnabled(false);
		StopFireLoop();
	}
}
