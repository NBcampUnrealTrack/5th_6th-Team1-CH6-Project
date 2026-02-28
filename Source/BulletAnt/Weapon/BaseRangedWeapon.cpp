#include "Weapon/BaseRangedWeapon.h"

#include "Weapon/Data/RangedWeaponDataAsset.h"

void ABaseRangedWeapon::BeginPlay()
{
	Super::BeginPlay();
	URangedWeaponDataAsset* Data = Cast<URangedWeaponDataAsset>(WeaponData);
	if (!Data) return;

	bAutoActive = Data->bAutoFire;
}
