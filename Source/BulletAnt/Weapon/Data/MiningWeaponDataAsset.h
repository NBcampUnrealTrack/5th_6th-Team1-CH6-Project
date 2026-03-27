#pragma once

#include "CoreMinimal.h"
#include "Weapon/Data/WeaponDataAsset.h"
#include "Mining/VoxelData.h"
#include "MiningWeaponDataAsset.generated.h"

UCLASS()
class BULLETANT_API UMiningWeaponDataAsset : public UWeaponDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly, meta = (ClampMin = 1.f));
	float TraceRadius = 20.f;

	UPROPERTY(EditDefaultsOnly, meta = (ClampMin = 1.f));
	float DigRadius = 240.f;

	UPROPERTY(EditDefaultsOnly, meta = (ClampMin = 1.f));
	float DigPerMinute = 180.f;

	UPROPERTY(EditDefaultsOnly);
	TMap<EOreType, float> OreEXPMap;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UAnimMontage> MiningMontage;

	UPROPERTY(EditDefaultsOnly)
	uint8 bAutoActive : 1 = true;
};
