// Fill out your copyright notice in the Description page of Project Settings.


#include "Building/BaseBuilding.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Net/UnrealNetwork.h"
#include "AbilitySystemComponent.h"
#include "GAS/AttributeSet/HealthAttributeSet.h"
#include "GeometryCollection/GeometryCollectionComponent.h"
#include "GeometryCollection/GeometryCollection.h"

ABaseBuilding::ABaseBuilding()
{
	bReplicates = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	StaticMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComp"));
	StaticMeshComp->SetupAttachment(RootComponent);

	StaticMeshComp->SetSimulatePhysics(false);
	StaticMeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	BuildingBounds = CreateDefaultSubobject<UBoxComponent>(TEXT("BuildingBounds"));
	BuildingBounds->SetupAttachment(RootComponent);

	BuildingBounds->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	BuildingBounds->SetCollisionResponseToAllChannels(ECR_Ignore);

	BuildingBounds->SetHiddenInGame(false);
	BuildingBounds->SetGenerateOverlapEvents(true);

	ASC = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("ASC"));
	ASC->SetIsReplicated(true);
	ASC->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);

	HealthSet = CreateDefaultSubobject<UHealthAttributeSet>(TEXT("HealthSet"));

	DestructionComp = CreateDefaultSubobject<UGeometryCollectionComponent>(TEXT("DestructionComp"));
	DestructionComp->SetupAttachment(RootComponent);
	DestructionComp->SetHiddenInGame(true);
	DestructionComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	DestructionComp->SetSimulatePhysics(false);
	DestructionComp->SetIsReplicated(false);
}

void ABaseBuilding::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABaseBuilding, BuildingBoxExtent);
}

UAbilitySystemComponent* ABaseBuilding::GetAbilitySystemComponent() const
{
	return ASC;
}

void ABaseBuilding::BeginPlay()
{
	Super::BeginPlay();

	if (DestructionCollection)
	{
		DestructionComp->SetRestCollection(DestructionCollection);
	}
}

void ABaseBuilding::OnDeath()
{
	if (!HasAuthority() || bDead)
	{
		return;
	}

	bDead = true;
	OnRep_Dead();
}

void ABaseBuilding::ApplyBuildingBounds(const FVector& InBoxExtent)
{
	BuildingBoxExtent = InBoxExtent;
	BuildingBounds->SetBoxExtent(BuildingBoxExtent);
	BuildingBounds->SetRelativeLocation(FVector(0.f, 0.f, BuildingBoxExtent.Z));
}


void ABaseBuilding::SetBuildingBoxExtent(const FVector& InBoxExtent)
{
	if (HasAuthority())
	{
		ApplyBuildingBounds(InBoxExtent);
	}
}

void ABaseBuilding::GetEdgesWorld(TArray<FBuildingEdge>& OutEdges) const
{
	TArray<FBuildingEdge> Local;
	GetEdgesLocal(Local);

	OutEdges.Reset(Local.Num());

	const FTransform T = GetActorTransform();
	for (const FBuildingEdge& E : Local)
	{
		FBuildingEdge W;
		W.A = T.TransformPosition(E.A);
		W.B = T.TransformPosition(E.B);
		OutEdges.Add(W);
	}
}

void ABaseBuilding::GetEdgesWorldWithTransform(const FTransform& T, TArray<FBuildingEdge>& OutEdges) const
{
	TArray<FBuildingEdge> Local;
	GetEdgesLocal(Local);

	OutEdges.Reset(Local.Num());
	for (const FBuildingEdge& E : Local)
	{
		FBuildingEdge W;
		W.A = T.TransformPosition(E.A);
		W.B = T.TransformPosition(E.B);
		OutEdges.Add(W);
	}
}

void ABaseBuilding::DrawEdgesDebug(bool bPersistentLines, float LifeTime) const
{
	UWorld* World = GetWorld();
	if (!World) return;

	TArray<FBuildingEdge> Edges;
	GetEdgesWorld(Edges);

	for (const FBuildingEdge& E : Edges)
	{
		DrawDebugLine(World, E.A, E.B, FColor::Cyan, bPersistentLines, LifeTime, 0, 2.f);
		DrawDebugSphere(World, E.Mid(), 10.f, 8, FColor::Cyan, bPersistentLines, LifeTime, 0, 1.f);
	}
}

void ABaseBuilding::GetEdgesLocal(TArray<FBuildingEdge>& OutEdges) const
{
	OutEdges.Reset();

	const float X = BuildingBoxExtent.X;
	const float Y = BuildingBoxExtent.Y;

	const FVector P0(+X, +Y, 0.f);
	const FVector P1(+X, -Y, 0.f);
	const FVector P2(-X, -Y, 0.f);
	const FVector P3(-X, +Y, 0.f);

	OutEdges.Add({ P0, P1 });
	OutEdges.Add({ P1, P2 });
	OutEdges.Add({ P2, P3 });
	OutEdges.Add({ P3, P0 });
}

void ABaseBuilding::OnRep_Dead()
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
}

void ABaseBuilding::Multicast_PlayDestruction_Implementation(const FVector& ImpulseOrigin)
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


void ABaseBuilding::OnRep_BuildingBoxExtent()
{
	ApplyBuildingBounds(BuildingBoxExtent);
}
