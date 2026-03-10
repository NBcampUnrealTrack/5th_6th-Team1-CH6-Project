#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GachaWeightData.generated.h"

USTRUCT(BlueprintType)
struct BULLETANT_API FGachaWeightData : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 GachaID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AActor> ActorClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Weight;
	
};
