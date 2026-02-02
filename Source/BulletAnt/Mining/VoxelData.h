#pragma once

#include "CoreMinimal.h"

struct FChunkMeshData
{
	TArray<FVector> Vertices;
	TArray<FIntVector> Triangles;
};

struct FChunkUpdateResult
{
	int32 UpdateID;
	int32 TargetChunkIdx;
	FChunkMeshData MeshData;
};
