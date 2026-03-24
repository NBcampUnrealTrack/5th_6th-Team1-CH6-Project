#include "Building/BaseShop.h" 

#include "Framework/BAGameMode.h"
#include "Framework/BAGameState.h"
#include "Shop/GachaCostData.h"
#include "Shop/GachaWeightData.h"
#include "Shop/BATransportShip.h"
#include "Mining/VoxelData.h"
#include "Player/BAPlayerController.h"
#include "GameFramework/Character.h"
#include "UI/UISubsystem.h"
#include "UI/UW_ShopWindow.h"
#include "Weapon/BaseWeapon.h"

ABaseShop::ABaseShop()
{

}

bool ABaseShop::CanBuyGacha(ABAPlayerController* PC, int32 InGachaID, int32 Count)
{
	ABAGameState* GS = Cast<ABAGameState>(GetWorld()->GetGameState());
	if (!GS) return false;

	if (!GachaCost) return false;
	if (!GachaWeight) return false;

	TMap<EOreType, int32> TotalCost;

	const TMap<EOreType, int32>& BaseCost = CachedCostData[InGachaID];

	for (const auto& Elem : BaseCost)
	{
		TotalCost.Add(Elem.Key, Elem.Value * Count);
	}

	if (GS->CanPurchase(TotalCost))
	{
		if (IsValid(PC))
		{
			PC->Server_RequestBuyGacha(this, InGachaID, Count);
			return true;
		}
	}

	return false;
}


void ABaseShop::BuyGacha(int32 InGachaID, int32 Count)
{
	if (!HasAuthority()) return;
	ABAGameMode* GM = Cast<ABAGameMode>(GetWorld()->GetAuthGameMode());
	if (!GM) return;

	TMap<EOreType, int32> TotalCost;

	const TMap<EOreType, int32>& BaseCost = CachedCostData[InGachaID];

	for (const auto& Elem : BaseCost)
	{
		TotalCost.Add(Elem.Key, Elem.Value * Count);
	}

	if (GM->TrySpendOre(TotalCost))
	{
		for (int32 i = 0; i < Count; i++)
		{
			TSubclassOf<ABaseWeapon> GachaOut = TryGacha(InGachaID);
			DropWeapon(GachaOut);
		}
	}
	else
	{

		return;
	}
}

void ABaseShop::DropWeapon(TSubclassOf<ABaseWeapon> InWeaponClass)
{
	if (!HasAuthority()) return;

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride =
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ABATransportShip* Ship = GetWorld()->SpawnActor<ABATransportShip>(
		TransportShipClass,
		GetActorLocation() + FVector(0.f, 0.f, 8000.f),
		FRotator::ZeroRotator,
		Params
	);

	FVector DropLocation = GetActorLocation() + FVector(0.f, 0.f, 8000.f);
	Ship->InitItemPlane(DropLocation, InWeaponClass);
}

void ABaseShop::ShowShop(ABAPlayerController* PC)
{
	ULocalPlayer* LP = PC->GetLocalPlayer();
	if (!LP) return;

	UISubsystem = LP->GetSubsystem<UUISubsystem>();
	if (IsValid(UISubsystem))
	{
		ShopWindow = UISubsystem->ShowUI<UUW_ShopWindow>(EUIType::Shop);
		UISubsystem->ApplyUIOnlyInputMode(ShopWindow);
		ShopWindow->InitShopUI(this);
	}
}

void ABaseShop::Use_Implementation(AActor* User)
{
	ACharacter* Character = Cast<ACharacter>(User);
	ABAPlayerController* PC = Cast<ABAPlayerController>(Character->GetController());
	if (!PC) return;

	ShowShop(PC);
}

void ABaseShop::GetInteractionOptions_Implementation(AActor* User, TArray<FInteractionOption>& OutOptions) const
{
	Super::GetInteractionOptions_Implementation(User, OutOptions);

	FInteractionOption Op1;
	Op1.Key = EKeys::F;
	Op1.Label = FText::FromString(TEXT("상점"));
	Op1.ActionName = TEXT("ShowShop");
	OutOptions.Add(Op1);
}

void ABaseShop::Interaction_Implementation(AActor* User, FName ActionName)
{
	if (ActionName == TEXT("ShowShop"))
	{
		Use_Implementation(User);
	}
	else
	{
		Super::Interaction_Implementation(User, ActionName);
	}
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

	for (FGachaCostData* Row : CostRow)
	{
		CachedCostData.FindOrAdd(Row->GachaID, Row->Cost);
	}
}

TSubclassOf<ABaseWeapon> ABaseShop::TryGacha(int32 InGachaID)
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
