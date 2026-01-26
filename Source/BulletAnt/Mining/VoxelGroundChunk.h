#pragma once

#include "CoreMinimal.h"
#include "Components/DynamicMeshComponent.h"
#include "VoxelGroundChunk.generated.h"

struct FNeighborLOD
{
	// X, Y, Z 방향 이웃의 LOD Level
	int32 XPlus, XMinus, YPlus, YMinus, ZPlus, ZMinus;

	// 생성자 기본값은 내 LOD로 해서 혹시 모를 상황 대비
	//FNeighborLOD(int32 MyLOD) : XPlus(MyLOD), XMinus(MyLOD), YPlus(MyLOD), YMinus(MyLOD), ZPlus(MyLOD), ZMinus(MyLOD) {}
};

struct FChunkMeshData
{
	TArray<FVector> Vertices;
	TArray<FIntVector> Triangles;
};

UCLASS()
class BULLETANT_API UVoxelGroundChunk : public UDynamicMeshComponent
{
	GENERATED_BODY()

public:
	void InitializeChunk(int32 InGridSize, float InVoxelSize, float InIsoLevel);
	void UpdateMeshAsync(const TArray<uint8>& DensityValues, const FNeighborLOD& NeighborLOD, int32 LODLevel = -1);

	FORCEINLINE int32 GetCurrentLODLevel() const { return CurrentLODLevel; }

	static void SetMaxLODLevel(int32 InMaxLODLevel);

protected:
	static FVector Interpolate(FVector P1, float V1, FVector P2, float V2, int32 InIsoLevel);
	static int32 GetIndex(int32 X, int32 Y, int32 Z, int32 InGridSize);
	void GenerateMesh(const FChunkMeshData& MeshData);

protected:
	int32 GridSize = 16;									// 복셀 갯수 - 청크의 한 변에 있는 복셀의 갯수
	float VoxelSize = 50.0f;								// 복셀 간격
	uint8 IsoLevel = 100;									// 지표면 임계값

	int32 CurrentLODLevel = -1;
	static int32 MaxLODLevel;
};
