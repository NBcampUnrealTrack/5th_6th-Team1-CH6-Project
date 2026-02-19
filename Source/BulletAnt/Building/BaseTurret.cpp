// Fill out your copyright notice in the Description page of Project Settings.


#include "Building/BaseTurret.h"
#include "GameplayTagContainer.h"
#include "Net/UnrealNetwork.h"
#include "AbilitySystemComponent.h"
#include "Weapon/Abilities/GA_Fire.h"
#include "Weapon/Data/RangedWeaponDataAsset.h"
#include "Components/SphereComponent.h"
#include "Enemy/BaseEnemy/BaseEnemyCharacter.h"

ABaseTurret::ABaseTurret()
{
	bReplicates = true;
	PrimaryActorTick.bCanEverTick = true;

	BodyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BodyMesh"));
	BodyMesh->SetupAttachment(StaticMeshComp);

	BarrelMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BarrelMesh"));
	BarrelMesh->SetupAttachment(BodyMesh);
	BarrelMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

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
		ASC->GiveAbility(FGameplayAbilitySpec(UGA_Fire::StaticClass(), 1));

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

	DOREPLIFETIME(ABaseTurret, CurrentTarget);
}

FVector ABaseTurret::GetFireStartLocation() const
{
	if (BarrelMesh && BarrelMesh->DoesSocketExist(MuzzleSocketName))
	{
		return BarrelMesh->GetSocketLocation(MuzzleSocketName);
	}

	return GetActorLocation();
}

FVector ABaseTurret::GetFireDirection() const
{
	if (BarrelMesh && BarrelMesh->DoesSocketExist(MuzzleSocketName))
	{
		const FTransform SocketTM = BarrelMesh->GetSocketTransform(MuzzleSocketName, RTS_World);
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
	Super::OnDeath();

	if (ASC && TurretData)
	{
		FGameplayTagContainer FireTags;
		FireTags.AddTag(TurretData->WeaponTag);
		ASC->CancelAbilities(&FireTags, nullptr, nullptr);
	}
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

void ABaseTurret::OnTargetBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor || OtherActor == this)
	{
		return;
	}

	ABaseEnemyCharacter* Enemy = Cast<ABaseEnemyCharacter>(OtherActor);
	if (!Enemy)
	{
		return;
	}
	TargetCandidates.AddUnique(Enemy);
}

void ABaseTurret::OnTargetEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	ABaseEnemyCharacter* Enemy = Cast<ABaseEnemyCharacter>(OtherActor);
	TargetCandidates.Remove(Enemy);

	if (CurrentTarget == OtherActor)
	{
		CurrentTarget = nullptr;
	}
}

void ABaseTurret::UpdateCurrentTarget()
{
	if (!HasAuthority() || bDead || !ASC || !TurretData)
	{
		return;
	}

	AActor* PrevTarget = CurrentTarget;

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

	FGameplayTagContainer FireTags;
	FireTags.AddTag(TurretData->WeaponTag);

	// 타겟 생김
	if (!IsValid(PrevTarget) && IsValid(CurrentTarget))
	{
		ASC->TryActivateAbilitiesByTag(FireTags);
	}
	// 타겟 잃음
	else if (!IsValid(CurrentTarget))
	{
		ASC->CancelAbilities(&FireTags);
	}
}
