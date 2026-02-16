#pragma once

#include "CoreMinimal.h"
#include "Weapon/Data/WeaponDataAsset.h"
#include "JetpackWeaponDataAsset.generated.h"

UCLASS()
class BULLETANT_API UJetpackWeaponDataAsset : public UWeaponDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly)
	uint8 bAutoActive : 1 = true;
};
