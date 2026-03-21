#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "MiningEffectMaterialMap.generated.h"

enum class EOreType : uint8;

UCLASS(BlueprintType)
class BULLETANT_API UMiningEffectMaterialMap : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TMap<EOreType, TObjectPtr<UMaterialInterface>> Map;
};
