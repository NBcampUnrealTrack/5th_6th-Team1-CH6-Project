#pragma once

#include "CoreMinimal.h"
#include "VoxelData.generated.h"

// 더 늘어나면 개념/담당 기준으로 분리

UENUM()
enum class EVoxelType : uint8
{
	BedRock,
	NormalRock,
	Vein,
	Pillar,

	None				UMETA(Hidden)
};

UENUM()
enum class EOreType
{
	Gold,
	Mineral,

	None				UMETA(Hidden)
};

struct FChunkMeshData
{
	TArray<FVector> Vertices;
	TArray<FVector4f> VertexColor;			// 이 값을 이용해서 머티리얼에서 텍스처 블렌딩
	TArray<FIntVector> Triangles;
};

struct FChunkUpdateResult
{
	int32 UpdateID;
	int32 TargetChunkIdx;
	FChunkMeshData MeshData;
};

USTRUCT()
struct FVoxelPointEditData
{
	GENERATED_BODY()

	UPROPERTY()
	int32 VoxelIndex = 0;
	UPROPERTY()
	uint8 NewDensityValue = 0;
};

USTRUCT()
struct FVoxelChunkEditData
{
	GENERATED_BODY()

	UPROPERTY()
	int32 ChunkIdx = -1;
	UPROPERTY()
	TArray<FVoxelPointEditData> PointEditDatas;

	UPROPERTY()
	int32 SendIdx = 0;			// Queue에서 전달할 때 사용
};

USTRUCT()
struct FVoxelChunkEditPacket
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FVoxelChunkEditData> ChunkEditDatas;
};

UENUM()
enum class EGroundType : uint8
{
	Default,

};

USTRUCT()
struct FGroundInitializeParams
{
	GENERATED_BODY()

	UPROPERTY()
	int32 Seed;
	UPROPERTY()
	EGroundType GroundType;
};

USTRUCT()
struct FBuryBoundInfo
{
	GENERATED_BODY()

	UPROPERTY()
	FTransform Transform;
	UPROPERTY()
	FVector Extent;
	UPROPERTY()
	FBox Bound;
};
