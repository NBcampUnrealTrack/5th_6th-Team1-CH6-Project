#include "Weapon/BaseJetpackWeapon.h"

#include "Weapon/Data/JetpackWeaponDataAsset.h"

void ABaseJetpackWeapon::BeginPlay()
{
	Super::BeginPlay();
	UJetpackWeaponDataAsset* Data = Cast<UJetpackWeaponDataAsset>(WeaponData);
	if (!Data) return;

	bAutoActive = Data->bAutoActive;
}
