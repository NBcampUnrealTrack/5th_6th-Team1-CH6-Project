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
	TArray<FIntVector> Triangles;
	TArray<uint8> MaterialIDs;
};

struct FChunkUpdateResult
{
	int32 UpdateID;
	int32 TargetChunkIdx;
	FChunkMeshData MeshData;
};
