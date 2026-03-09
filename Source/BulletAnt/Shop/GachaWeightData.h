#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GachaWeightData.generated.h"

class ABaseWeapon;

USTRUCT()
struct BULLETANT_API FGachaWeightData : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 GachaID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<ABaseWeapon> WeaponClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Weight;
	
};
