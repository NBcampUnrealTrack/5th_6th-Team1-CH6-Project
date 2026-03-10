#include "Building/BaseShop.h" 

#include "Framework/BAGameMode.h"
#include "Framework/BAGameState.h"
#include "Shop/GachaCostData.h"
#include "Shop/GachaWeightData.h"
#include "Shop/BATransportShip.h"
#include "Mining/VoxelData.h"

ABaseShop::ABaseShop()
{

}

void ABaseShop::CanBuyGacha(int32 InGachaID)
{
	ABAGameState* GS = Cast<ABAGameState>(GetWorld()->GetGameState());
	if (!GS) return;

	if (!GachaCost) return;
	if (!GachaWeight) return;

	if (GS->CanPurchase(CachedCostData[InGachaID]))
	{
		Server_BuyGacha(InGachaID);
	}
	else
	{

		return;
	}
}

void ABaseShop::Server_BuyGacha_Implementation(int32 InGachaID)
{
	ABAGameMode* GM = Cast<ABAGameMode>(GetWorld()->GetAuthGameMode());
	if (!GM) return;

	if (!GachaCost) return;
	if (!GachaWeight) return;

	TSubclassOf<AActor> GachaOut = TryGacha(InGachaID);

	if (GM->TrySpendOre(CachedCostData[InGachaID]))
	{
		DropItem(GachaOut);
	}
	else
	{
		
		return;
	}
}

void ABaseShop::ShowGacha()
{

}

void ABaseShop::DropItem(TSubclassOf<AActor> InActor)
{
	if (!HasAuthority()) return;

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride =
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ABATransportShip* Ship = GetWorld()->SpawnActor<ABATransportShip>(
		TransportShipClass,
		GetActorLocation() + FVector(0.f, 0.f, 5000.f),
		FRotator::ZeroRotator,
		Params
	);

	FVector DropLocation = GetActorLocation()+FVector(0.f, 0.f, 5000.f);
	Ship->InitPlane(DropLocation, InActor);
}

void ABaseShop::ShowWeapon()
{
	
}

void ABaseShop::Use_Implementation(AActor* User)
{
	CanBuyGacha(1);
}

void ABaseShop::BeginPlay()
{
	Super::BeginPlay();

	if (!GachaCost) return;
	if (!GachaWeight) return;

	TArray<FGachaWeightData*> WeightRow;
	GachaWeight->GetAllRows(TEXT("GachaWeight"), WeightRow);
	for (FGachaWeightData* Row : WeightRow)
	{
		CachedWeightData.FindOrAdd(Row->GachaID).Add(*Row);
	}
	
	TArray<FGachaCostData*> CostRow;
	GachaCost->GetAllRows(TEXT("GachaCost"), CostRow);

	for (FGachaCostData *Row : CostRow)
	{
		CachedCostData.FindOrAdd(Row->GachaID, Row->Cost);
	}
}

TSubclassOf<AActor> ABaseShop::TryGacha(int32 InGachaID)
{
	if (!GachaWeight) return nullptr;


	float TotalWeight = 0;

	for (FGachaWeightData Row : CachedWeightData[InGachaID])
	{
		if (Row.GachaID == InGachaID)
		{
			TotalWeight += Row.Weight;
		}
	}

	float Rand = FMath::RandRange(0.f, TotalWeight);

	for (FGachaWeightData Row : CachedWeightData[InGachaID])
	{
		if (Row.GachaID == InGachaID)
		{
			Rand -= Row.Weight;
			if (Rand <= 0)
			{
				return Row.ActorClass;
			}
		}	
	}

	return nullptr;
}
