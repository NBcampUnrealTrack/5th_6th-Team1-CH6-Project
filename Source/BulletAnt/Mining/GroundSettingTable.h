#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Mining/VoxelData.h"
#include "GroundSettingTable.generated.h"

class UGroundSettingPreset;

UCLASS()
class BULLETANT_API UGroundSettingTable : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TMap<EGroundType, TObjectPtr<const UGroundSettingPreset>> Settings;
};
