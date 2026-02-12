#pragma once

#include "CoreMinimal.h"
#include "Weapon/Data/WeaponDataAsset.h"
#include "MiningWeaponDataAsset.generated.h"

UCLASS()
class BULLETANT_API UMiningWeaponDataAsset : public UWeaponDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly)
	float DigPerMinute = 180.f;

	UPROPERTY(EditDefaultsOnly)
	uint8 bAutoFire : 1 = true;
};
