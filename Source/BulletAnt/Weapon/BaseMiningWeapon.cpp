#include "Weapon/BaseMiningWeapon.h"

#include "Weapon/Data/MiningWeaponDataAsset.h"

void ABaseMiningWeapon::BeginPlay()
{
	Super::BeginPlay();
	UMiningWeaponDataAsset* Data = Cast<UMiningWeaponDataAsset>(WeaponData);
	if (!Data) return;

	bAutoActive = Data->bAutoActive;
}
