#pragma once

#include "CoreMinimal.h"
#include "Building/BaseBuilding.h"
#include "Common/BAItemInterface.h"
#include "Mining/VoxelData.h"
#include "Shop/GachaWeightData.h"
#include "BaseShop.generated.h"

class ABaseWeapon;
class ABATransportShip;

UCLASS()
class BULLETANT_API ABaseShop : public ABaseBuilding, public IBAItemInterface
{
	GENERATED_BODY()
	
public:
	ABaseShop();

	UFUNCTION()
	void CanBuyGacha(int32 InGachaID);

	UFUNCTION(Server, Reliable)
	void Server_BuyGacha(int32 InGachaID);

	UFUNCTION()
	void ShowGacha();

	UFUNCTION()
	void DropItem(TSubclassOf<AActor> InActor);

	UFUNCTION()
	void ShowWeapon();
	
	virtual void Use_Implementation(AActor* User) override;

protected:
	virtual void BeginPlay() override;

	TSubclassOf<AActor> TryGacha(int32 InGachaID);

	UPROPERTY()
	TArray<TSubclassOf<ABaseWeapon>> WeaponArray;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Data|Weight")
	TObjectPtr<UDataTable> GachaWeight;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Data|Cost")
	TObjectPtr<UDataTable> GachaCost;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Plane")
	TSubclassOf<ABATransportShip> TransportShipClass;

	TMap<int32, TArray<FGachaWeightData>> CachedWeightData;
	TMap<int32, TMap<EOreType, int32>> CachedCostData;
};
