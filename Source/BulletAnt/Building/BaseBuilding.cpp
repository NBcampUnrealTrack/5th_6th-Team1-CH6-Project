// Fill out your copyright notice in the Description page of Project Settings.


#include "Building/BaseBuilding.h"
#include "Components/StaticMeshComponent.h"
#include "Net/UnrealNetwork.h"
#include "AbilitySystemComponent.h"
#include "GAS/AttributeSet/HealthAttributeSet.h"
#include "GAS/Ability/GA_DestroyBuilding.h"
#include "GeometryCollection/GeometryCollectionComponent.h"
#include "GeometryCollection/GeometryCollection.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SplineComponent.h"

ABaseBuilding::ABaseBuilding()
{
	bReplicates = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	EdgesRoot = CreateDefaultSubobject<USceneComponent>(TEXT("EdgesRoot"));
	EdgesRoot->SetupAttachment(RootComponent);

	PlacementRoot = CreateDefaultSubobject<USceneComponent>(TEXT("PlacementRoot"));
	PlacementRoot->SetupAttachment(RootComponent);

	StaticMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComp"));
	StaticMeshComp->SetupAttachment(RootComponent);

	StaticMeshComp->SetSimulatePhysics(false);
	StaticMeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

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

	PreviewBaseMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/BulletAnt/Building/M_BuildingPreview.M_BuildingPreview"));
}

void ABaseBuilding::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABaseBuilding, bDead);
}

UAbilitySystemComponent* ABaseBuilding::GetAbilitySystemComponent() const
{
	return ASC;
}

void ABaseBuilding::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		ASC->GiveAbility(FGameplayAbilitySpec(UGA_DestroyBuilding::StaticClass(), 1));
		HealthSet->SetMaxHealth(500.f);
		HealthSet->SetHealth(500.f);
	}

	if (DestructionCollection)
	{
		DestructionComp->SetRestCollection(DestructionCollection);
	}

	RebuildCachedLocalEdges();
}

void ABaseBuilding::OnDeath()
{
	if (!HasAuthority() || bDead)
	{
		return;
	}

	Multicast_PlayDestruction(GetActorLocation());
	bDead = true;
	OnRep_Dead();
	SetLifeSpan(DebrisLifeSeconds);
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

void ABaseBuilding::RebuildCachedLocalEdges()
{
	CachedLocalEdges.Reset();

	if (!EdgesRoot)
	{
		return;
	}

	TArray<USceneComponent*> EdgeComps;
	EdgesRoot->GetChildrenComponents(true, EdgeComps);

	const FTransform WorldToActor = GetActorTransform().Inverse();

	for (USceneComponent* EdgeComp : EdgeComps)
	{
		if (USplineComponent* Edge = Cast<USplineComponent>(EdgeComp))
		{
			if (!Edge) 
			{
				continue;
			}

			if (Edge->GetNumberOfSplinePoints() < 2)
			{
				continue;
			}

			FVector WA = Edge->GetLocationAtSplinePoint(0, ESplineCoordinateSpace::World);
			FVector WB = Edge->GetLocationAtSplinePoint(1, ESplineCoordinateSpace::World);

			FBuildingEdge E;
			E.A = WorldToActor.TransformPosition(WA);
			E.B = WorldToActor.TransformPosition(WB);

			CachedLocalEdges.Add(E);
		}
	}
}

void ABaseBuilding::GetPlacementPrimitives(TArray<UPrimitiveComponent*>& OutPrims) const
{
	OutPrims.Reset();
	if (!PlacementRoot) 
	{
		return;
	}

	TArray<USceneComponent*> PrimComps;
	PlacementRoot->GetChildrenComponents(true, PrimComps);

	for (USceneComponent* PrimComp : PrimComps)
	{
		if (UPrimitiveComponent* Prim = Cast<UPrimitiveComponent>(PrimComp))
		{
			if (Prim->GetCollisionEnabled() != ECollisionEnabled::NoCollision &&
				Prim->GetGenerateOverlapEvents())
			{
				OutPrims.Add(Prim);
			}
		}
	}
}

void ABaseBuilding::SetPreviewMode(bool bInPreview)
{
	bPreviewMode = bInPreview;

	if (StaticMeshComp)
	{
		StaticMeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	TArray<UPrimitiveComponent*> Prims;
	GetPlacementPrimitives(Prims);

	for (UPrimitiveComponent* Prim : Prims)
	{
		if (!Prim) 
		{
			continue;
		}

		Prim->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		Prim->SetGenerateOverlapEvents(true);
		Prim->SetCollisionObjectType(ECC_GameTraceChannel1);
		Prim->SetCollisionResponseToAllChannels(ECR_Ignore);
		Prim->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Overlap);
		Prim->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
		Prim->UpdateOverlaps();
	}

	SetActorTickEnabled(!bPreviewMode);

	if (bPreviewMode && PreviewBaseMaterial && StaticMeshComp)
	{
		PreviewMID = UMaterialInstanceDynamic::Create(PreviewBaseMaterial, this);
		const int32 NumMaterials = StaticMeshComp->GetNumMaterials();
		for (int32 i = 0; i < NumMaterials; ++i)
		{
			StaticMeshComp->SetMaterial(i, PreviewMID);
		}
	}

	if (bPreviewMode)
	{
		SetCanPlace(false);
	}
}

void ABaseBuilding::SetCanPlace(bool bInCanPlace)
{
	if (!bPreviewMode) 
	{
		return;
	}

	if (!PreviewMID)
	{
		return;
	}

	const float Opacity = 0.7f;
	const FLinearColor CanColor(0.f, 1.f, 0.f, Opacity);
	const FLinearColor BlockColor(1.f, 0.f, 0.f, Opacity);

	PreviewMID->SetVectorParameterValue(TEXT("ActorColor"), bInCanPlace ? CanColor : BlockColor);
}

void ABaseBuilding::GetEdgesLocal(TArray<FBuildingEdge>& OutEdges) const
{
	OutEdges = CachedLocalEdges;
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
