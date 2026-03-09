#pragma once

#include "CoreMinimal.h"
#include "Building/BaseBuilding.h"
#include "BaseShop.generated.h"

class ABaseWeapon;


UCLASS()
class BULLETANT_API ABaseShop : public ABaseBuilding
{
	GENERATED_BODY()
	
public:
	ABaseShop();

	UFUNCTION(Server, Reliable)
	void Server_BuyGacha(int32 GachaID);

	UFUNCTION()
	void ShowGacha();

	UFUNCTION()
	void DropWeapon();

	UFUNCTION()
	void ShowWeapon();

protected:
	UPROPERTY()
	TArray<TSubclassOf<ABaseWeapon>> WeaponArray;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Data|Weight")
	TObjectPtr<UDataTable> GachaWeight;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Data|Cost")
	TObjectPtr<UDataTable> GachaCost;
};
