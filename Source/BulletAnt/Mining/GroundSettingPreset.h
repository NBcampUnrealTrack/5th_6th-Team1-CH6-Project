#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Mining/VoxelData.h"
#include "GroundSettingPreset.generated.h"

UCLASS()
class BULLETANT_API UGroundSettingPreset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GroundSetting")
	FVector GroundSize = FVector(40000.0f, 40000.0f, 60000.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GroundSetting")
	int32 ChunkGridSize = 32;			// 복셀 갯수 - 청크의 한 변에 있는 복셀의 갯수
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GroundSetting")
	float ChunkVoxelSize = 100.0f;		// 복셀 간격
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GroundSetting")
	uint8 IsoLevel = 100;				// 지표면 임계값

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "OreSetting")
	EVoxelType VeinVoxelType;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "OreSetting")
	EVoxelType PillarVoxelType;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GroundMaterial")
	TObjectPtr<UMaterialInterface> GroundMaterial;
};
