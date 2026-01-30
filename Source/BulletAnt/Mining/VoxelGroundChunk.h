#pragma once

#include "CoreMinimal.h"
#include "Components/DynamicMeshComponent.h"
#include "VoxelGroundChunk.generated.h"

struct FNeighborLOD
{
	// 면 방향 : XNeg, XPos, YNeg, YPos, ZNeg, ZPos
	int32 LODs[6];
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
	void InitializeChunk(int32 InGridSize, float InVoxelSize, uint8 InIsoLevel);
	void UpdateMeshAsync(const TArray<uint8>& DensityValues, const FNeighborLOD& NeighborLOD, int32 LODLevel = -1);

	FORCEINLINE int32 GetCurrentLODLevel() const { return CurrentLODLevel; }

	static void SetMaxLODLevel(int32 InMaxLODLevel);

protected:
	static FVector Interpolate(FVector P1, float V1, FVector P2, float V2, uint8 InIsoLevel);
	static int32 GetIndex(int32 X, int32 Y, int32 Z, int32 InGridSize);
	static int32 GetIndex(const FIntVector& Vec, int32 InGridSize);
	static int32 GetStepByLODLevel(int32 LODLevel);

	static void GenerateRegularCell(
		int32 X, int32 Y, int32 Z,
		int32 LocalLODLevel, int32 Step,
		uint8 NeighborMask,
		FChunkMeshData& MeshData, 
		TMap<FVector, int32>& VertexCache,
		const TArray<uint8>& LocalDensity,
		float LocalVoxelSize,
		int32 LocalGridSize,
		uint8 LocalIsoLevel);

	static void GenerateTransitionCell(
		int32 FaceIdx,
		int32 X, int32 Y, int32 Z,
		int32 LocalLODLevel, int32 Step,
		uint8 NeighborMask,
		FChunkMeshData& MeshData,
		TMap<FVector, int32>& VertexCache,
		const TArray<uint8>& LocalDensity,
		float LocalVoxelSize,
		int32 LocalGridSize,
		uint8 LocalIsoLevel);

	// TransitionCell을 위해 안쪽으로 수축된 가상의 정점 위치
	// TransitionCell이 있는 RegularCell에서는 안쪽의 정점들은 해당 위치를 이용해야 함.
	static FVector GetVirtualPosition(FVector PrimaryPos, int32 LODIndex, uint8 NeighborMask, float LocalVoxelSize, int32 LocalGridSize);

	static FIntVector GetSwizzledPos(int32 FaceIdx, int32 U, int32 V, int32 Step);
	void GenerateMesh(const FChunkMeshData& MeshData);

protected:
	int32 GridSize = 16;									// 복셀 갯수 - 청크의 한 변에 있는 복셀의 갯수
	float VoxelSize = 50.0f;								// 복셀 간격
	uint8 IsoLevel = 100;									// 지표면 임계값

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 CurrentLODLevel = -1;
	static int32 MaxLODLevel;

public:
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly)
	FIntVector CoordV;					// 임시
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 ChunkIdxV;					// 임시
};
