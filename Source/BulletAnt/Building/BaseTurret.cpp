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

	if (DestructionCollection)
	{
		DestructionComp->SetRestCollection(DestructionCollection);
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

void ABaseTurret::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABaseTurret, bDead);
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

	if (StaticMeshComp)
	{
		StaticMeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		StaticMeshComp->SetHiddenInGame(true);
	}

	Multicast_PlayDestruction(GetActorLocation());
	UE_LOG(LogTemp, Warning, TEXT("ABaseTurret::OnDeath"));
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

void ABaseTurret::Multicast_PlayDestruction_Implementation(const FVector& ImpulseOrigin)
{
	if (!DestructionComp || !DestructionCollection)
	{
		return;
	}

	// 죽는 순간 위치를 현재 터렛 위치에 맞춤
	const FTransform SpawnTM = StaticMeshComp ? StaticMeshComp->GetComponentTransform()
		: GetActorTransform();

	// 먼저 월드 트랜스폼 세팅
	DestructionComp->SetWorldTransform(SpawnTM);

	// 붙어있으면 물리가 제대로 안 도는 경우가 많아서 분리
	DestructionComp->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);

	// 렌더 켬
	DestructionComp->SetHiddenInGame(false, true);
	DestructionComp->SetVisibility(true, true);

	// 충돌/물리 켬
	DestructionComp->SetMobility(EComponentMobility::Movable);
	DestructionComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	DestructionComp->SetSimulatePhysics(false); // 한번 끔
	DestructionComp->SetEnableGravity(true);

	// 바운드/렌더/물리 상태 강제 갱신 (안 보이다가 갑자기 나타나는 문제 해결)
	DestructionComp->UpdateBounds();
	DestructionComp->MarkRenderStateDirty();
	DestructionComp->RecreatePhysicsState();

	// 시뮬 켬
	DestructionComp->SetSimulatePhysics(true);
	DestructionComp->WakeAllRigidBodies();

	// 임펄스 테스트용
	const float Strength = 80000.f;
	FVector Dir = (GetActorLocation() - ImpulseOrigin);
	Dir = Dir.IsNearlyZero() ? FVector(1, 0, 1).GetSafeNormal() : Dir.GetSafeNormal();

	DestructionComp->AddImpulse(Dir * Strength, NAME_None, true);
}
