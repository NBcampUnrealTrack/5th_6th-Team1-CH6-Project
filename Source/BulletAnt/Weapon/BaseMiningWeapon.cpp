#include "Weapon/BaseMiningWeapon.h"

#include "Weapon/Data/MiningWeaponDataAsset.h"

void ABaseMiningWeapon::BeginPlay()
{
	Super::BeginPlay();
	UMiningWeaponDataAsset* Data = Cast<UMiningWeaponDataAsset>(WeaponData);

	bAutoActive = Data->bAutoActive;
}
