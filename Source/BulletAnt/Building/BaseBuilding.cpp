// Fill out your copyright notice in the Description page of Project Settings.


#include "Building/BaseBuilding.h"
#include "Components/StaticMeshComponent.h"
#include "Net/UnrealNetwork.h"

ABaseBuilding::ABaseBuilding()
{
	StaticMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComp"));
	SetRootComponent(StaticMeshComp);

	StaticMeshComp->SetSimulatePhysics(false);
	StaticMeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	bReplicates = true;
}

void ABaseBuilding::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABaseBuilding, PlacementBoxExtent);
}

void ABaseBuilding::SetPlacementBoxExtent(const FVector& InBoxExtent)
{
	if (HasAuthority())
	{
		PlacementBoxExtent = InBoxExtent;
	}

	DrawDebugBox(
		GetWorld(),
		GetActorLocation(),
		PlacementBoxExtent,
		GetActorQuat(),
		FColor::Cyan,
		true,
		-1.f,
		0,
		2.f
	);
}
