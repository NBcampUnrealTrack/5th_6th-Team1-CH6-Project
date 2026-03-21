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
#include "Components/BoxComponent.h"
#include "Building/BuildManagerComponent.h"
#include "Player/BACharacter.h"

ABaseBuilding::ABaseBuilding()
{
	bReplicates = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	EdgesRoot = CreateDefaultSubobject<USceneComponent>(TEXT("EdgesRoot"));
	EdgesRoot->SetupAttachment(RootComponent);

	PlacementRoot = CreateDefaultSubobject<USceneComponent>(TEXT("PlacementRoot"));
	PlacementRoot->SetupAttachment(RootComponent);

	SupportRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SupportRoot"));
	SupportRoot->SetupAttachment(RootComponent);

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
	DestructionComp->SetCanEverAffectNavigation(false);
	DestructionComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	DestructionComp->SetCollisionResponseToChannel(ECC_GameTraceChannel6, ECR_Ignore);

	PreviewBaseMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/BulletAnt/Building/M_BuildingPreview.M_BuildingPreview"));
}

void ABaseBuilding::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (!StaticMeshComp || !StaticMeshComp->GetStaticMesh())
	{
		return;
	}

	FVector Min, Max;
	StaticMeshComp->GetLocalBounds(Min, Max);
	FVector LocalCenter = (Min + Max) * 0.5f;
	LocalCenter.Z = Min.Z;

	StaticMeshComp->SetRelativeLocation(-LocalCenter);
	EdgesRoot->SetRelativeLocation(-LocalCenter);
	PlacementRoot->SetRelativeLocation(-LocalCenter);
	SupportRoot->SetRelativeLocation(-LocalCenter);

	RebuildCachedLocalEdges();
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
		HealthSet->SetMaxHealth(DefaultHealth);
		HealthSet->SetHealth(DefaultHealth);
	}

	if (DestructionCollection)
	{
		DestructionComp->SetRestCollection(DestructionCollection);
	}

	RebuildCachedLocalEdges();
}

void ABaseBuilding::Use_Implementation(AActor* User)
{
	if (!IsValid(User))
	{
		return;
	}

	ABACharacter* Character = Cast<ABACharacter>(User);
	if (!Character)
	{
		return;
	}

	UBuildManagerComponent* BuildManager = Character->FindComponentByClass<UBuildManagerComponent>();
	if (!BuildManager)
	{
		return;
	}

	BuildManager->RequestDemolish(this);
}

void ABaseBuilding::RequestDemolish(AActor* User)
{
	if (!HasAuthority() || bDead)
	{
		return;
	}

	OnDeath();
}

void ABaseBuilding::OnDeath()
{
	if (!HasAuthority() || bDead)
	{
		return;
	}

	TArray<TWeakObjectPtr<ABaseBuilding>> Above;
	Above.Reserve(SupportedBuildings.Num());
	for (const auto& Building : SupportedBuildings)
	{
		Above.Add(Building);
	}

	Multicast_PlayDestruction(GetActorLocation());
	bDead = true;
	OnRep_Dead();
	SetLifeSpan(DebrisLifeSeconds);

	Server_UnregisterFromSupports();

	SupportedBuildings.Reset();

	for (const auto& Building : Above)
	{
		if (ABaseBuilding* B = Building.Get())
		{
			B->Server_ReevaluateSupportAndMaybeDie();
		}
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

void ABaseBuilding::GetSupportVolumes(TArray<UPrimitiveComponent*>& OutVolumes) const
{
	OutVolumes.Reset();
	if (!SupportRoot) 
	{
		return;
	}

	TArray<USceneComponent*> SupportVol;
	SupportRoot->GetChildrenComponents(true, SupportVol);

	for (USceneComponent* Support : SupportVol)
	{
		if (UPrimitiveComponent* P = Cast<UPrimitiveComponent>(Support))
		{
			OutVolumes.Add(P);
		}
	}
}

bool ABaseBuilding::ComputeSupportCoverage(const FTransform& WorldT, float& OutCoverage, TSet<TWeakObjectPtr<ABaseBuilding>>& OutSupportBuildings) const
{
	OutSupportBuildings.Reset();
	OutCoverage = 0.f;

	UWorld* World = GetWorld();
	if (!World) 
	{
		return false;
	}

	TArray<UPrimitiveComponent*> SupportVolumes;
	GetSupportVolumes(SupportVolumes);

	if (SupportVolumes.Num() == 0)
	{
		GetPlacementPrimitives(SupportVolumes);
	}

	if (SupportVolumes.Num() == 0)
	{
		return false;
	}

	FBox UnionBox(EForceInit::ForceInit);

	for (UPrimitiveComponent* P : SupportVolumes)
	{
		if (!P) 
		{
			continue;
		}

		const FTransform CompWorldT = P->GetRelativeTransform() * WorldT;
		const FBoxSphereBounds B = P->CalcBounds(CompWorldT);
		UnionBox += B.GetBox();
	}

	if (!UnionBox.IsValid) 
	{
		return false;
	}

	const FVector Center = UnionBox.GetCenter();
	const FVector Ext = UnionBox.GetExtent();

	const float MinX = Center.X - Ext.X;
	const float MaxX = Center.X + Ext.X;
	const float MinY = Center.Y - Ext.Y;
	const float MaxY = Center.Y + Ext.Y;

	const float Step = FMath::Max(10.f, SupportSampleSpacing);

	int32 Total = 0;
	int32 Supported = 0;

	FCollisionQueryParams Params(SCENE_QUERY_STAT(SupportTrace), false);
	Params.AddIgnoredActor(this);

	FCollisionObjectQueryParams Obj;
	Obj.AddObjectTypesToQuery(ECC_WorldStatic);
	Obj.AddObjectTypesToQuery(ECC_GameTraceChannel1);

	const FName GroundActorTag = TEXT("Ground");

	const float SampleZ = UnionBox.Min.Z + 5.f;
	for (float X = MinX; X <= MaxX; X += Step)
	{
		for (float Y = MinY; Y <= MaxY; Y += Step)
		{
			Total++;
			const FVector Sample(X, Y, SampleZ);
			const FVector Start = Sample + FVector(0, 0, SupportTraceUp);
			const FVector End = Sample - FVector(0, 0, SupportTraceDown);

			FHitResult Hit;
			if (!World->LineTraceSingleByObjectType(Hit, Start, End, Obj, Params))
			{
				continue;
			}

			AActor* HitActor = Hit.GetActor();
			UPrimitiveComponent* HitComp = Hit.GetComponent();
			if (!HitActor || !HitComp) 
			{
				continue;
			}

			if (HitComp->GetCollisionObjectType() == ECC_WorldStatic || HitActor->ActorHasTag(GroundActorTag))
			{
				Supported++;
				continue;
			}

			if (ABaseBuilding* Building = Cast<ABaseBuilding>(HitActor))
			{
				if (!Building->bDead)
				{
					Supported++;
					OutSupportBuildings.Add(Building);
				}
			}
		}
	}

	if (Total <= 0)
	{
		return false;
	}
	OutCoverage = (float)Supported / (float)Total;
	return true;
}

void ABaseBuilding::Server_RegisterSupports(const TSet<TWeakObjectPtr<ABaseBuilding>>& Supporters)
{
	if (!HasAuthority() || bDead)
	{
		return;
	}

	Server_UnregisterFromSupports();

	SupportingBuildings = Supporters;

	for (const TWeakObjectPtr<ABaseBuilding>& Building : SupportingBuildings)
	{
		if (ABaseBuilding* Supporter = Building.Get())
		{
			Supporter->SupportedBuildings.Add(this);
		}
	}
}

void ABaseBuilding::Server_UnregisterFromSupports()
{
	if (!HasAuthority())
	{
		return;
	}

	for (const TWeakObjectPtr<ABaseBuilding>& Building : SupportingBuildings)
	{
		if (ABaseBuilding* Supporter = Building.Get())
		{
			Supporter->SupportedBuildings.Remove(this);
		}
	}
	SupportingBuildings.Reset();
}

void ABaseBuilding::Server_ReevaluateSupportAndMaybeDie()
{
	if (!HasAuthority() || bDead)
	{
		return;
	}

	float Coverage = 0.f;
	TSet<TWeakObjectPtr<ABaseBuilding>> SupportsNow;

	if (!ComputeSupportCoverage(GetActorTransform(), Coverage, SupportsNow))
	{
		OnDeath();
		return;
	}

	Server_RegisterSupports(SupportsNow);

	if (Coverage < MinSupportCoverage)
	{
		OnDeath();
	}
}

void ABaseBuilding::ApplyBuildingRow(const FBuildingRow& Row)
{
	MinSupportCoverage = Row.MinSupportCoverage;
	SupportSampleSpacing = Row.SupportSampleSpacing;
	DefaultHealth = Row.Health;
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
		StaticMeshComp->SetCanEverAffectNavigation(false);
	}

	TArray<UActorComponent*> Components;
	GetComponents(UPrimitiveComponent::StaticClass(), Components);

	for (UActorComponent* Comp : Components)
	{
		UPrimitiveComponent* Prim = Cast<UPrimitiveComponent>(Comp);
		if (!Prim)
		{
			continue;
		}

		if (Prim == DestructionComp)
		{
			continue;
		}

		Prim->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Prim->SetGenerateOverlapEvents(false);
		Prim->SetCanEverAffectNavigation(false);
	}

	UBoxComponent* Box = FindComponentByClass<UBoxComponent>();
	if (IsValid(Box))
	{
		Box->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	OnDestroyed.Broadcast();
}

void ABaseBuilding::Multicast_PlayDestruction_Implementation(const FVector& ImpulseOrigin)
{
	if (!DestructionComp || !DestructionCollection)
	{
		return;
	}

	// 렌더 켬
	DestructionComp->SetHiddenInGame(false);

	// 네비 영향 off
	DestructionComp->SetCanEverAffectNavigation(false);

	// 충돌/물리 켬
	DestructionComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	DestructionComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	DestructionComp->SetCollisionResponseToChannel(ECC_GameTraceChannel6, ECR_Ignore);

	// 시뮬 켬
	DestructionComp->SetSimulatePhysics(true);

	// 임펄스
	const float Strength = 500.f;
	FVector Dir = (GetActorLocation() - ImpulseOrigin);
	Dir = Dir.IsNearlyZero() ? FVector(1, 0, 1).GetSafeNormal() : Dir.GetSafeNormal();

	DestructionComp->AddImpulse(Dir * Strength, NAME_None, true);
}
