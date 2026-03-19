#pragma once

#include "CoreMinimal.h"
#include "Building/BaseBuilding.h"
#include "Common/BAItemInterface.h"
#include "Mining/VoxelData.h"
#include "Shop/GachaWeightData.h"
#include "BaseShop.generated.h"

class ABaseWeapon;
class ABATransportShip;
class ABAPlayerController;
class UUW_ShopWindow;
class UUISubsystem;

UCLASS()
class BULLETANT_API ABaseShop : public ABaseBuilding
{
	GENERATED_BODY()
	
public:
	ABaseShop();

	UFUNCTION()
	bool CanBuyGacha(ABAPlayerController* PC, int32 InGachaID, int32 Count);

	UFUNCTION()
	void BuyGacha(int32 InGachaID, int32 Count);

	UFUNCTION()
	void DropWeapon(TSubclassOf<ABaseWeapon> InWeapon);

	UFUNCTION()
	void ShowShop(ABAPlayerController* PC);
	
	virtual void Use_Implementation(AActor* User) override;

	const TMap<int32, TMap<EOreType, int32>>& GetCachedCostData() const { return CachedCostData; };

protected:
	virtual void BeginPlay() override;

	TSubclassOf<ABaseWeapon> TryGacha(int32 InGachaID);

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

	UPROPERTY()
	UUW_ShopWindow* ShopWindow;

	UPROPERTY()
	UUISubsystem* UISubsystem;

};
