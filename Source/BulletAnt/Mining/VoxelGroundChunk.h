#pragma once

#include "CoreMinimal.h"
#include "Components/DynamicMeshComponent.h"
#include "VoxelGroundChunk.generated.h"

UCLASS()
class BULLETANT_API UVoxelGroundChunk : public UDynamicMeshComponent
{
	GENERATED_BODY()

public:
	void InitializeChunk(int32 InGridSize, float InVoxelSize, float InIsoLevel);
	void UpdateMesh(const TArray<uint8>& DensityValues);
	FVector Interpolate(FVector P1, float V1, FVector P2, float V2) const;
	int32 GetIndex(int32 X, int32 Y, int32 Z) const;

protected:
	int32 GridSize = 16;									// 복셀 갯수 - 청크의 한 변에 있는 복셀의 갯수
	float VoxelSize = 50.0f;								// 복셀 간격
	uint8 IsoLevel = 100;									// 지표면 임계값
};
