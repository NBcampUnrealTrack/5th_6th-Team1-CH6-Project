#include "Weapon/BaseRangedWeapon.h"

#include "Weapon/Data/RangedWeaponDataAsset.h"

void ABaseRangedWeapon::BeginPlay()
{
	Super::BeginPlay();
	Data = Cast<URangedWeaponDataAsset>(WeaponData);

	bAutoActive = Data->bAutoFire;
}

