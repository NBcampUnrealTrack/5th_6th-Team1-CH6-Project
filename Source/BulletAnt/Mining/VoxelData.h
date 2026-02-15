#pragma once

#include "CoreMinimal.h"

UENUM()
enum class EVoxelType : uint8
{
	BedRock,
	NormalRock,
	Gold,
	Mineral,

	None				UMETA(Hidden)
};

struct FChunkMeshData
{
	TArray<FVector> Vertices;
	TArray<FVector4f> VertexColor;			// 이 값을 이용해서 머티리얼에서 텍스처 블렌딩
	TArray<FIntVector> Triangles;
	TArray<uint8> MaterialIDs;
};

struct FChunkUpdateResult
{
	int32 UpdateID;
	int32 TargetChunkIdx;
	FChunkMeshData MeshData;
};
