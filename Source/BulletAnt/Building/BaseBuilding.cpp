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

void ABaseBuilding::OnRep_BuildingBoxExtent()
{
	ApplyBuildingBounds(BuildingBoxExtent);
}
