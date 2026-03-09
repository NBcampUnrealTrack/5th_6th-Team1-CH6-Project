#include "Building/BaseShop.h" 

#include "Framework/BAGameMode.h"
#include "Shop/GachaCostData.h"
#include "Shop/GachaWeightData.h"

ABaseShop::ABaseShop()
{

}

void ABaseShop::Server_BuyGacha_Implementation(int32 GachaID)
{
	
	ABAGameMode* GM = Cast<ABAGameMode>(GetWorld()->GetAuthGameMode());

}

void ABaseShop::ShowGacha()
{

}

void ABaseShop::DropWeapon()
{

}

void ABaseShop::ShowWeapon()
{
	
}