#include "Mining/VoxelGroundChunk.h"
#include "Mining/MarchingCubeTable.h"

int32 UVoxelGroundChunk::MaxLODLevel = 2;

void UVoxelGroundChunk::InitializeChunk(int32 InGridSize, float InVoxelSize, float InIsoLevel)
{
	GridSize = InGridSize;
	VoxelSize = InVoxelSize;
	IsoLevel = InIsoLevel;
}

void UVoxelGroundChunk::UpdateMeshAsync(const TArray<uint8>& DensityValues, const FNeighborLOD& NeighborLOD, int32 LODLevel)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UpdateMeshAsync);
	if (LODLevel >= 0 && CurrentLODLevel == LODLevel)
		return;

	if (DensityValues.IsEmpty() == true)
		return;

	if (LODLevel >= 0)
	{
		CurrentLODLevel = LODLevel;
	}

	TArray<uint8> LocalDensity = DensityValues;
	TWeakObjectPtr<UVoxelGroundChunk> WeakThis(this);
	FVector PlanetCenter = GetOwner()->GetActorLocation();
	FVector ChunkWorldPos = GetComponentLocation();

	// 쓰레드 안전을 위해 멤버변수도 복사해서 전달
	// []안에 넣으면 값 복사되는 줄 알았는데 컴파일러가 this포인터를 캡쳐해서 멤버변수에 접근하게 된다고 함
	int32 LocalLODLevel = CurrentLODLevel;
	int32 LocalVoxelSize = VoxelSize;
	int32 LocalGridSize = GridSize;
	int32 LocalIsoLevel = IsoLevel;

	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask,
		[WeakThis, LocalDensity, NeighborLOD, PlanetCenter, ChunkWorldPos, LocalLODLevel, LocalVoxelSize, LocalGridSize, LocalIsoLevel]()
		{
			FChunkMeshData MeshData;
			TMap<FVector, int32> VertexCache;

			// LODLevel에 따라 멀리 있는 청크면 더 많은 갯수만큼 건너뛰면서 체크 (LOD0: 1, LOD1: 2, LOD2: 4)
			int32 Step = FMath::Pow((double)2, LocalLODLevel);
			const int32 MaxStep = FMath::Pow((double)2, MaxLODLevel);
			float GridSnapUnit = MaxStep * LocalVoxelSize;

			for (int32 Z = 0; Z < LocalGridSize; Z += Step)
			{
				for (int32 Y = 0; Y < LocalGridSize; Y += Step)
				{
					for (int32 X = 0; X < LocalGridSize; X += Step)
					{
						FVector Pos[8]{};
						uint8 Density[8]{};
						for (int32 Idx = 0; Idx < 8; ++Idx)
						{
							FIntVector Corner(X + CornerTable[Idx][0] * Step, Y + CornerTable[Idx][1] * Step, Z + CornerTable[Idx][2] * Step);
							Pos[Idx] = FVector(Corner.X, Corner.Y, Corner.Z) * LocalVoxelSize;
							Density[Idx] = LocalDensity[GetIndex(Corner.X, Corner.Y, Corner.Z, LocalGridSize)];
						}

						int32 CubeIdx = 0;
						for (int Idx = 0; Idx < 8; ++Idx)
						{
							if (Density[Idx] > LocalIsoLevel)
							{
								CubeIdx |= (1 << Idx);
							}
						}

						if (CubeIdx == 0 || CubeIdx == 255)
							continue;

						for (int32 Idx = 0; TriangleTable[CubeIdx][Idx] != -1; Idx += 3)
						{
							FIntVector NewTriangle;
							for (int32 TriangleIdx = 0; TriangleIdx < 3; ++TriangleIdx)
							{
								int32 EdgeIdx = TriangleTable[CubeIdx][Idx + TriangleIdx];

								int32 E1 = EdgeIndexTable[EdgeIdx][0];
								int32 E2 = EdgeIndexTable[EdgeIdx][1];

								// 버텍스의 위치를 보간해서 너무 각지지 않고 부드러워 보이게 함
								FVector VertexPos = Interpolate(Pos[E1], Density[E1], Pos[E2], Density[E2], LocalIsoLevel);

								int32* CachedIdx = VertexCache.Find(VertexPos);
								if (CachedIdx != nullptr)
								{
									NewTriangle[TriangleIdx] = *CachedIdx;
								}
								else
								{
									int32 VertexIdx = MeshData.Vertices.Add(VertexPos);
									VertexCache.Add(VertexPos, VertexIdx);
									NewTriangle[TriangleIdx] = VertexIdx;
								}
							}

							MeshData.Triangles.Add(NewTriangle);
						}
					}
				}
			}

			AsyncTask(ENamedThreads::GameThread,
				[WeakThis, MeshData]()
				{
					if (WeakThis.IsValid() == true)
					{
						WeakThis->GenerateMesh(MeshData);
					}
				});
		});
}


FVector UVoxelGroundChunk::Interpolate(FVector P1, float V1, FVector P2, float V2, int32 InIsoLevel)
{
	if (FMath::Abs(V1 - V2) < 0.00001f)
		return P1;

	return P1 + (InIsoLevel - V1) / (V2 - V1) * (P2 - P1);
}

int32 UVoxelGroundChunk::GetIndex(int32 X, int32 Y, int32 Z, int32 InGridSize)
{
	return Z * (InGridSize + 1) * (InGridSize + 1) + Y * (InGridSize + 1) + X;
}

void UVoxelGroundChunk::GenerateMesh(const FChunkMeshData& MeshData)
{
	UDynamicMesh* Mesh = GetDynamicMesh();
	if (IsValid(Mesh) == false)
		return;

	FDynamicMesh3& DynamicMesh3 = Mesh->GetMeshRef();
	DynamicMesh3.Clear();

	for (const FVector& Vertex : MeshData.Vertices)
	{
		DynamicMesh3.AppendVertex(Vertex);
	}

	for (const FIntVector& Triangle : MeshData.Triangles)
	{
		DynamicMesh3.AppendTriangle(Triangle.X, Triangle.Y, Triangle.Z);
	}

	// 렌더링 업데이트
	NotifyMeshModified();
	// 충돌체 업데이트
	UpdateCollision(false);

	UpdateBounds();
}

void UVoxelGroundChunk::SetMaxLODLevel(int32 InMaxLODLevel)
{
	MaxLODLevel = InMaxLODLevel;
}
