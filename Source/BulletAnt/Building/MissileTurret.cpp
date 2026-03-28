#include "Building/MissileTurret.h"

#include "AbilitySystemComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Building/RangedTurretDataAsset.h"
#include "Weapon/Data/RangedWeaponDataAsset.h"

AMissileTurret::AMissileTurret()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AMissileTurret::BeginPlay()
{
	Super::BeginPlay();

	BuildMissileVisuals();

	if (HasAuthority())
	{
		StartLaunchSolutionTimer();
	}
}

void AMissileTurret::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}

void AMissileTurret::UpdateAim(float DeltaSeconds)
{
	UpdateMissileAim(DeltaSeconds);
}

void AMissileTurret::ApplyPreviewMode()
{
	Super::ApplyPreviewMode();

	if (!bPreviewMode || !PreviewMID)
	{
		return;
	}

	for (UStaticMeshComponent* VisualComp : MissileVisuals)
	{
		if (!VisualComp)
		{
			continue;
		}

		const int32 NumMaterials = VisualComp->GetNumMaterials();
		for (int32 MatIdx = 0; MatIdx < NumMaterials; ++MatIdx)
		{
			VisualComp->SetMaterial(MatIdx, PreviewMID);
		}
	}
}

float AMissileTurret::GetAttackInterval() const
{
	if (!RangedTurretData || !RangedTurretData->WeaponData || RangedTurretData->WeaponData->RoundPerMinute <= 0.f)
	{
		return 0.f;
	}

	// RPM = 전탄 발사 + 전탄 재장전 전체 사이클
	return 60.f / RangedTurretData->WeaponData->RoundPerMinute;
}

float AMissileTurret::GetCycleDuration() const
{
	return GetAttackInterval();
}

float AMissileTurret::GetFireStepInterval() const
{
	const int32 MuzzleCount = MuzzleSocketNames.Num();
	if (MuzzleCount <= 1)
	{
		return 0.f;
	}

	const float FirePhaseDuration = GetCycleDuration() * FirePhaseRatio;
	return FirePhaseDuration / float(MuzzleCount - 1);
}

float AMissileTurret::GetReloadStepInterval() const
{
	const int32 MuzzleCount = MuzzleSocketNames.Num();
	if (MuzzleCount <= 1)
	{
		return 0.f;
	}

	const float ReloadPhaseDuration = GetCycleDuration() * (1.f - FirePhaseRatio);
	return ReloadPhaseDuration / float(MuzzleCount - 1);
}

void AMissileTurret::ExecuteAttack()
{
	if (!HasAuthority() || bDead || !ASC || !CurrentTarget || bCycleInProgress)
	{
		return;
	}

	if (MuzzleSocketNames.IsEmpty())
	{
		return;
	}

	// 장전 안 된 슬롯이 있으면 사이클 시작하지 않음
	for (bool bLoaded : bMissileLoaded)
	{
		if (!bLoaded)
		{
			return;
		}
	}

	bCycleInProgress = true;
	SequenceIndex = 0;

	StartFireSequence();
}

void AMissileTurret::StartFireSequence()
{
	SequenceIndex = 0;
	FireSequenceStep();
}

void AMissileTurret::FireSequenceStep()
{
	if (!HasAuthority() || !ASC || !CurrentTarget)
	{
		bCycleInProgress = false;
		return;
	}

	const int32 MuzzleCount = MuzzleSocketNames.Num();
	if (!MuzzleSocketNames.IsValidIndex(SequenceIndex))
	{
		StartReloadSequence();
		return;
	}

	CurrentMuzzleIndex = SequenceIndex;

	if (RefreshLaunchSolution())
	{
		FGameplayTagContainer FireTags;
		FireTags.AddTag(RangedTurretData->WeaponData->WeaponTag);
		ASC->TryActivateAbilitiesByTag(FireTags);

		SetMissileVisualLoaded(SequenceIndex, false);
	}

	++SequenceIndex;

	if (SequenceIndex < MuzzleCount)
	{
		GetWorldTimerManager().SetTimer(
			SequenceTimerHandle,
			this,
			&AMissileTurret::FireSequenceStep,
			GetFireStepInterval(),
			false
		);
	}
	else
	{
		StartReloadSequence();
	}
}

void AMissileTurret::StartReloadSequence()
{
	SequenceIndex = 0;
	ReloadSequenceStep();
}

void AMissileTurret::ReloadSequenceStep()
{
	const int32 MuzzleCount = MuzzleSocketNames.Num();

	if (!MuzzleSocketNames.IsValidIndex(SequenceIndex))
	{
		bCycleInProgress = false;
		return;
	}

	SetMissileVisualLoaded(SequenceIndex, true);

	++SequenceIndex;

	if (SequenceIndex < MuzzleCount)
	{
		GetWorldTimerManager().SetTimer(
			SequenceTimerHandle,
			this,
			&AMissileTurret::ReloadSequenceStep,
			GetReloadStepInterval(),
			false
		);
	}
	else
	{
		bCycleInProgress = false;
	}
}

bool AMissileTurret::PrepareLaunchSolution()
{
	return RefreshLaunchSolution();
}

void AMissileTurret::UpdateMissileAim(float DeltaSeconds)
{
	if (!CurrentTarget || !BodyMesh || !BarrelMesh)
	{
		return;
	}

	if (!bHasValidLaunchSolution)
	{
		return;
	}

	const FVector ToTarget = CurrentTarget->GetActorLocation() - BodyMesh->GetComponentLocation();
	if (ToTarget.IsNearlyZero())
	{
		return;
	}

	const FRotator TargetRot = ToTarget.Rotation();

	const float CurrentYaw = BodyMesh->GetComponentRotation().Yaw;
	const float NewYaw = FMath::FixedTurn(
		CurrentYaw,
		TargetRot.Yaw,
		TurretData ? TurretData->TurnSpeedDegPerSec * DeltaSeconds : 360.f * DeltaSeconds
	);

	BodyMesh->SetWorldRotation(FRotator(0.f, NewYaw, 0.f));

	const FRotator LaunchWorldRot = CachedLaunchDirection.Rotation();
	const FRotator CurrentBodyRot = BodyMesh->GetComponentRotation();
	const FRotator RelativeRot = (LaunchWorldRot - CurrentBodyRot).GetNormalized();

	const float CurrentPitch = BarrelMesh->GetRelativeRotation().Pitch;
	const float TargetPitch = RelativeRot.Pitch;
	const float NewPitch = FMath::FixedTurn(
		CurrentPitch,
		TargetPitch,
		TurretData ? TurretData->TurnSpeedDegPerSec * DeltaSeconds : 360.f * DeltaSeconds
	);

	BarrelMesh->SetRelativeRotation(FRotator(NewPitch, 0.f, 0.f));
}

bool AMissileTurret::RefreshLaunchSolution()
{
	bHasValidLaunchSolution = false;

	if (!IsValid(CurrentTarget) || !RangedTurretData || !RangedTurretData->WeaponData || !BarrelMesh)
	{
		return false;
	}

	const FVector Start = GetFireStartLocation_Implementation();
	const FVector TargetPoint = CurrentTarget->GetActorLocation();

	FVector LaunchVelocity;
	const bool bSuccess = UGameplayStatics::SuggestProjectileVelocity(
		this,
		LaunchVelocity,
		Start,
		TargetPoint,
		RangedTurretData->WeaponData->ProjectileSpeed,
		true,
		0.f,
		0.f,
		ESuggestProjVelocityTraceOption::DoNotTrace
	);

	if (!bSuccess || LaunchVelocity.IsNearlyZero())
	{
		return false;
	}

	CachedLaunchDirection = LaunchVelocity.GetSafeNormal();
	CachedImpactPoint = TargetPoint;
	CachedSolutionTarget = CurrentTarget;
	CachedSolutionTargetLocation = TargetPoint;
	bHasValidLaunchSolution = true;

	return true;
}

void AMissileTurret::StartLaunchSolutionTimer()
{
	if (!HasAuthority())
	{
		return;
	}

	GetWorldTimerManager().SetTimer(
		LaunchSolutionTimerHandle,
		this,
		&AMissileTurret::RefreshLaunchSolutionTick,
		GetAttackInterval() * 0.9f,
		true,
		FMath::FRandRange(0.f, GetAttackInterval() * 0.9f)
	);
}

void AMissileTurret::StopLaunchSolutionTimer()
{
	if (LaunchSolutionTimerHandle.IsValid())
	{
		GetWorldTimerManager().ClearTimer(LaunchSolutionTimerHandle);
		LaunchSolutionTimerHandle.Invalidate();
	}
}

void AMissileTurret::RefreshLaunchSolutionTick()
{
	if (!HasAuthority() || bDead)
	{
		return;
	}

	if (!IsCurrentTargetStillValid())
	{
		bHasValidLaunchSolution = false;
		CachedSolutionTarget.Reset();
		return;
	}

	if (!CurrentTarget)
	{
		bHasValidLaunchSolution = false;
		CachedSolutionTarget.Reset();
		return;
	}

	const FVector TargetLoc = CurrentTarget->GetActorLocation();

	const bool bTargetChanged = CachedSolutionTarget.Get() != CurrentTarget;
	const bool bMovedFarEnough =
		FVector::DistSquared(TargetLoc, CachedSolutionTargetLocation) >
		FMath::Square(100.f);

	if (bTargetChanged || bMovedFarEnough || !bHasValidLaunchSolution)
	{
		RefreshLaunchSolution();
	}
}

bool AMissileTurret::IsCurrentTargetStillValid() const
{
	if (!IsValid(CurrentTarget) || !TurretData)
	{
		return false;
	}

	const float DistSq = FVector::DistSquared(GetActorLocation(), CurrentTarget->GetActorLocation());
	return DistSq <= FMath::Square(TurretData->SearchRadius);
}

void AMissileTurret::BuildMissileVisuals()
{
	MissileVisuals.Empty();
	bMissileLoaded.Empty();

	if (!BarrelMesh || !MissileVisualMesh || MuzzleSocketNames.IsEmpty())
	{
		return;
	}

	for (int32 i = 0; i < MuzzleSocketNames.Num(); ++i)
	{
		if (!MuzzleSocketNames.IsValidIndex(i))
		{
			continue;
		}

		const FName SocketName = MuzzleSocketNames[i];
		if (!BarrelMesh->DoesSocketExist(SocketName))
		{
			continue;
		}

		UStaticMeshComponent* VisualComp = NewObject<UStaticMeshComponent>(this);
		if (!VisualComp)
		{
			continue;
		}

		VisualComp->SetStaticMesh(MissileVisualMesh);
		VisualComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		VisualComp->SetGenerateOverlapEvents(false);
		VisualComp->SetCanEverAffectNavigation(false);

		VisualComp->SetupAttachment(BarrelMesh, SocketName);
		VisualComp->RegisterComponent();

		VisualComp->SetRelativeLocation(FVector::ZeroVector);
		VisualComp->SetRelativeRotation(FRotator::ZeroRotator);

		MissileVisuals.Add(VisualComp);
		bMissileLoaded.Add(true);
	}
}

void AMissileTurret::SetMissileVisualLoaded(int32 Index, bool bLoaded)
{
	if (bMissileLoaded.IsValidIndex(Index))
	{
		bMissileLoaded[Index] = bLoaded;
	}

	if (MissileVisuals.IsValidIndex(Index) && MissileVisuals[Index])
	{
		MissileVisuals[Index]->SetHiddenInGame(!bLoaded);
		MissileVisuals[Index]->SetVisibility(bLoaded);
	}
}