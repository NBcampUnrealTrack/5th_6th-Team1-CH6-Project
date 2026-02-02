#pragma once

#include "CoreMinimal.h"
#include "Components/DynamicMeshComponent.h"
#include "Mining/VoxelData.h"
#include "VoxelGroundChunk.generated.h"

class AVoxelGround;

struct FNeighborLOD
{
	// 면 방향 : XNeg, XPos, YNeg, YPos, ZNeg, ZPos
	int32 LODs[6];
};

struct FVoxelGenerationContext
{
	const TArray<uint8>& Density;
	int32 LODLevel;
	int32 Step;
	float VoxelSize;
	int32 GridSize;
	uint8 IsoLevel;
	uint8 NeighborMask;

	FChunkMeshData& OutMeshData;
	TMap<uint64, int32>& OutVertexCache;

	FVoxelGenerationContext(
		const TArray<uint8>& InDensity,
		int32 InLODLevel, int32 InStep,
		float InVoxelSize,
		int32 InGridSize,
		uint8 InIsoLevel,
		uint8 InNeighborMask,
		FChunkMeshData& InMeshData,
		TMap<uint64, int32>& InVertexCache)
		: Density(InDensity)
		, LODLevel(InLODLevel)
		, Step(InStep)
		, VoxelSize(InVoxelSize)
		, GridSize(InGridSize)
		, IsoLevel(InIsoLevel)
		, NeighborMask(InNeighborMask)
		, OutMeshData(InMeshData)
		, OutVertexCache(InVertexCache) {}
};

UCLASS()
class BULLETANT_API UVoxelGroundChunk : public UDynamicMeshComponent
{
	GENERATED_BODY()

public:
	void InitializeChunk(int32 InGridSize, float InVoxelSize, uint8 InIsoLevel);
	void CalculateMeshDataAsync(int32 UpdateID, AVoxelGround* VoxelGround, const TArray<uint8>& DensityValues, const FNeighborLOD& NeighborLOD, int32 LODLevel = -1);
	void UpdateChunk(int32 UpdateID, const FChunkMeshData& MeshData, bool bUpdatePhysics = false);

	FORCEINLINE int32 GetCurrentLODLevel() const { return CurrentLODLevel; }
	FORCEINLINE int32 GetLastUpdateID() const { return LastUpdateID; }

	static void SetMaxLODLevel(int32 InMaxLODLevel);

protected:
	static FVector Interpolate(FVector P1, float V1, FVector P2, float V2, uint8 InIsoLevel);
	static int32 GetIndex(int32 X, int32 Y, int32 Z, int32 InGridSize);
	static int32 GetIndex(const FIntVector& Vec, int32 InGridSize);
	static int32 GetStepByLODLevel(int32 LODLevel);

	static uint32 GetVertexInfoForKey(int32 X, int32 Y, int32 Z, uint8 ShrinkDir);
	static uint64 GetVertexKey(uint32 VertexInfo0, uint32 VertexInfo1);

	static void GenerateRegularCell(
		int32 X, int32 Y, int32 Z,
		const FVoxelGenerationContext& Context);

	static void GenerateTransitionCell(
		int32 FaceIdx,
		int32 X, int32 Y, int32 Z,
		const FVoxelGenerationContext& Context);

	// TransitionCell을 위해 안쪽으로 수축된 가상의 정점 위치
	// TransitionCell이 있는 RegularCell에서는 안쪽의 정점들은 해당 위치를 이용해야 함.
	static uint8 GetAdjustedPosition(const FVector& PrimaryPos, FVector& OutFinalPos, int32 LODIndex, uint8 NeighborMask, float LocalVoxelSize, int32 LocalGridSize);

	static FIntVector GetSwizzledPos(int32 FaceIdx, int32 U, int32 V, int32 Step);

protected:
	int32 GridSize = 16;									// 복셀 갯수 - 청크의 한 변에 있는 복셀의 갯수
	float VoxelSize = 50.0f;								// 복셀 간격
	uint8 IsoLevel = 100;									// 지표면 임계값

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 CurrentLODLevel = -1;
	static int32 MaxLODLevel;

	int32 LastUpdateID = -1;

public:
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly)
	FIntVector CoordV;					// 임시
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 ChunkIdxV;					// 임시
};
