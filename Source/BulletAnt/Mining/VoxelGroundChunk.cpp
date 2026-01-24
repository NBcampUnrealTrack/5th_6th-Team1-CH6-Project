#include "Mining/VoxelGroundChunk.h"
#include "Mining/MarchingCubeTable.h"

void UVoxelGroundChunk::InitializeChunk(int32 InGridSize, float InVoxelSize, float InIsoLevel)
{
	GridSize = InGridSize;
	VoxelSize = InVoxelSize;
	IsoLevel = InIsoLevel;
}

void UVoxelGroundChunk::UpdateMesh(const TArray<uint8>& DensityValues)
{
	if (DensityValues.IsEmpty() == true)
		return;

	UDynamicMesh* Mesh = GetDynamicMesh();
	if (IsValid(Mesh) == false)
		return;

	FDynamicMesh3& MeshData = Mesh->GetMeshRef();
	MeshData.Clear();
	MeshData.EnableAttributes();

	TMap<FVector, int32> VertexCache;

	FVector PlanetCenter = GetOwner()->GetActorLocation();
	FVector ChunkWorldPos = GetComponentLocation();

	for (int32 Z = 0; Z < GridSize; ++Z)
	{
		for (int32 Y = 0; Y < GridSize; ++Y)
		{
			for (int32 X = 0; X < GridSize; ++X)
			{
				FVector Pos[8]{};
				uint8 Density[8]{};
				for (int32 Idx = 0; Idx < 8; ++Idx)
				{
					FIntVector Corner(X + CornerTable[Idx][0], Y + CornerTable[Idx][1], Z + CornerTable[Idx][2]);
					Pos[Idx] = FVector(Corner.X, Corner.Y, Corner.Z) * VoxelSize;
					Density[Idx] = DensityValues[GetIndex(Corner.X, Corner.Y, Corner.Z)];
				}

				int32 CubeIdx = 0;
				for (int Idx = 0; Idx < 8; ++Idx)
				{
					if (Density[Idx] > IsoLevel)
					{
						CubeIdx |= (1 << Idx);
					}
				}

				if (CubeIdx == 0 || CubeIdx == 255)
					continue;

				for (int32 Idx = 0; TriangleTable[CubeIdx][Idx] != -1; Idx += 3)
				{
					int32 NewTriangle[3] = {};
					for (int32 TriangleIdx = 0; TriangleIdx < 3; ++TriangleIdx)
					{
						int32 EdgeIdx = TriangleTable[CubeIdx][Idx + TriangleIdx];

						int32 E1 = EdgeIndexTable[EdgeIdx][0];
						int32 E2 = EdgeIndexTable[EdgeIdx][1];

						// 버텍스의 위치를 보간해서 너무 각지지 않고 부드러워 보이게 함
						FVector VertexPos = Interpolate(Pos[E1], Density[E1], Pos[E2], Density[E2]);
						int32* CachedIdx = VertexCache.Find(VertexPos);
						if (CachedIdx != nullptr)
						{
							NewTriangle[TriangleIdx] = *CachedIdx;
						}
						else
						{
							int32 VertexIdx = MeshData.AppendVertex(VertexPos);
							VertexCache.Add(VertexPos, VertexIdx);
							NewTriangle[TriangleIdx] = VertexIdx;
						}
					}

					MeshData.AppendTriangle(NewTriangle[0], NewTriangle[1], NewTriangle[2]);
				}
			}
		}
	}

	// 렌더링 업데이트
	NotifyMeshModified();
	// 충돌체 업데이트
	UpdateCollision(false);
	
	UpdateBounds();
}

FVector UVoxelGroundChunk::Interpolate(FVector P1, float V1, FVector P2, float V2) const
{
	if (FMath::Abs(V1 - V2) < 0.00001f)
		return P1;

	return P1 + (IsoLevel - V1) / (V2 - V1) * (P2 - P1);
}

int32 UVoxelGroundChunk::GetIndex(int32 X, int32 Y, int32 Z) const
{
	return Z * (GridSize + 1) * (GridSize + 1) + Y * (GridSize + 1) + X;
}
