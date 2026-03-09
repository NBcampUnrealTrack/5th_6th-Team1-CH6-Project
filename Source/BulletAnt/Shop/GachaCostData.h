#pragma once

#include "CoreMinimal.h"
#include "Mining/VoxelData.h"
#include "Engine/DataTable.h"
#include "GachaCostData.generated.h"

USTRUCT(BlueprintType)
struct BULLETANT_API FGachaCostData : public FTableRowBase
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 GachaID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<EOreType, int32> Cost;
};
