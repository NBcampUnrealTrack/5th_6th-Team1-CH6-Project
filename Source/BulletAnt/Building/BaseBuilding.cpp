// Fill out your copyright notice in the Description page of Project Settings.


#include "Building/BaseBuilding.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Net/UnrealNetwork.h"

ABaseBuilding::ABaseBuilding()
{
	bReplicates = true;

	StaticMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComp"));
	SetRootComponent(StaticMeshComp);

	StaticMeshComp->SetSimulatePhysics(false);
	StaticMeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	BuildingBounds = CreateDefaultSubobject<UBoxComponent>(TEXT("BuildingBounds"));
	BuildingBounds->SetupAttachment(RootComponent);

	BuildingBounds->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	BuildingBounds->SetCollisionResponseToAllChannels(ECR_Ignore);

	BuildingBounds->SetHiddenInGame(false);
	BuildingBounds->SetGenerateOverlapEvents(true);
}

void ABaseBuilding::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABaseBuilding, BuildingBoxExtent);
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

void ABaseBuilding::GetSnapPointsWorld(TArray<FVector>& OutPoints) const
{
	TArray<FVector> Local;
	GetSnapPointsLocal(Local);

	OutPoints.Reset(Local.Num());

	const FTransform T = GetActorTransform();
	for (const FVector& P : Local)
	{
		OutPoints.Add(T.TransformPosition(P));
	}
}

void ABaseBuilding::DrawSnapPointsDebug(bool bPersistentLines, float LifeTime) const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	TArray<FVector> SnapPoints;
	GetSnapPointsWorld(SnapPoints);

	for (const FVector& P : SnapPoints)
	{
		DrawDebugSphere(World, P, 12.f, 8, FColor::Yellow, bPersistentLines, LifeTime, 0, 1.5f);
	}
}

void ABaseBuilding::OnRep_BuildingBoxExtent()
{
	ApplyBuildingBounds(BuildingBoxExtent);
}

void ABaseBuilding::GetSnapPointsLocal(TArray<FVector>& OutPoints) const
{
	OutPoints.Reset();

	const float X = BuildingBoxExtent.X;
	const float Y = BuildingBoxExtent.Y;

	// 바닥 4면 꼭지점
	OutPoints.Add(FVector(+X, +Y, 0.f));
	OutPoints.Add(FVector(+X, -Y, 0.f));
	OutPoints.Add(FVector(-X, +Y, 0.f));
	OutPoints.Add(FVector(-X, -Y, 0.f));
}
