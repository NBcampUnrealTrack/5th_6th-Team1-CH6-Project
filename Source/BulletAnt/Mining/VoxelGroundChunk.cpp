#include "Mining/VoxelGroundChunk.h"
#include "Mining/MarchingCubeTable.h"

int32 UVoxelGroundChunk::MaxLODLevel = 2;

void UVoxelGroundChunk::InitializeChunk(int32 InGridSize, float InVoxelSize, uint8 InIsoLevel)
{
	GridSize = InGridSize;
	VoxelSize = InVoxelSize;
	IsoLevel = InIsoLevel;
}

void UVoxelGroundChunk::UpdateMeshAsync(const TArray<uint8>& DensityValues, const FNeighborLOD& NeighborLOD, int32 LODLevel)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UpdateMeshAsync);

	if (DensityValues.IsEmpty() == true)
		return;

	if (LODLevel >= 0)
	{
		CurrentLODLevel = LODLevel;
	}

	TArray<uint8> LocalDensity = DensityValues;
	TWeakObjectPtr<UVoxelGroundChunk> WeakThis(this);

	// 쓰레드 안전을 위해 멤버변수도 복사해서 전달
	// []안에 넣으면 값 복사되는 줄 알았는데 컴파일러가 this포인터를 캡쳐해서 멤버변수에 접근하게 된다고 함
	int32 LocalLODLevel = CurrentLODLevel;
	float LocalVoxelSize = VoxelSize;
	int32 LocalGridSize = GridSize;
	uint8 LocalIsoLevel = IsoLevel;

	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask,
		[WeakThis, LocalDensity, NeighborLOD, LocalLODLevel, LocalVoxelSize, LocalGridSize, LocalIsoLevel, this]()
		{
			FChunkMeshData MeshData;
			TMap<FVector, int32> VertexCache;

			int32 Step = GetStepByLODLevel(LocalLODLevel);

			int32 MaxSize = LocalGridSize - Step;

			// -X, +X, -Y, +Y, -Z, +Z 방향 TransitionCell 존재 여부
			uint8 NeighborMask = 0;
			for (int32 Idx = 0; Idx < 6; ++Idx)
			{
				if (LocalLODLevel > NeighborLOD.LODs[Idx])
				{
					NeighborMask |= (1 << Idx);
				}
			}

			auto CallGenerateRegular = [&](int32 X, int32 Y, int32 Z)
				{
					GenerateRegularCell(X, Y, Z, LocalLODLevel, Step, NeighborMask, MeshData, VertexCache, LocalDensity, LocalVoxelSize, LocalGridSize, LocalIsoLevel);
				};

			auto CallGenerateTransition = [&](int32 X, int32 Y, int32 Z, int32 FaceIdx)
				{
					if (NeighborMask & (1 << FaceIdx))
					{
						GenerateTransitionCell(FaceIdx, X, Y, Z, LocalLODLevel, Step, NeighborMask, MeshData, VertexCache, LocalDensity, LocalVoxelSize, LocalGridSize, LocalIsoLevel);
					}
				};

			// 내부 RegularCell
			for (int32 Z = 0; Z <= MaxSize; Z += Step)
			{
				for (int32 Y = 0; Y <= MaxSize; Y += Step)
				{
					for (int32 X = 0; X <= MaxSize; X += Step)
					{
						CallGenerateRegular(X, Y, Z);

						if (X == 0)
						{
							CallGenerateTransition(X, Y, Z, 0);
						}

						if (X == MaxSize)
						{
							CallGenerateTransition(X, Y, Z, 1);
						}

						if (Y == 0)
						{
							CallGenerateTransition(X, Y, Z, 2);
						}

						if (Y == MaxSize)
						{
							CallGenerateTransition(X, Y, Z, 3);
						}

						if (Z == 0)
						{
							CallGenerateTransition(X, Y, Z, 4);
						}

						if (Z == MaxSize)
						{
							CallGenerateTransition(X, Y, Z, 5);
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


FVector UVoxelGroundChunk::Interpolate(FVector P1, float V1, FVector P2, float V2, uint8 InIsoLevel)
{
	if (FMath::Abs(V1 - V2) < 0.00001f)
		return P1;

	return FMath::Lerp(P1, P2, (InIsoLevel - V1) / (V2 - V1));
}

int32 UVoxelGroundChunk::GetIndex(int32 X, int32 Y, int32 Z, int32 InGridSize)
{
	return Z * (InGridSize + 1) * (InGridSize + 1) + Y * (InGridSize + 1) + X;
}

int32 UVoxelGroundChunk::GetIndex(const FIntVector& Vec, int32 InGridSize)
{
	return GetIndex(Vec.X, Vec.Y, Vec.Z, InGridSize);
}

int32 UVoxelGroundChunk::GetStepByLODLevel(int32 LODLevel)
{
	// LODLevel에 따라 멀리 있는 청크면 더 많은 갯수만큼 건너뛰면서 체크 (LOD0: 1, LOD1: 2, LOD2: 4)
	return 1 << LODLevel;;
}

void UVoxelGroundChunk::GenerateRegularCell(int32 X, int32 Y, int32 Z, int32 LocalLODLevel, int32 Step, uint8 NeighborMask, FChunkMeshData& MeshData, TMap<FVector, int32>& VertexCache, const TArray<uint8>& LocalDensity, float LocalVoxelSize, int32 LocalGridSize, uint8 LocalIsoLevel)
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
	for (int32 Idx = 0; Idx < 8; ++Idx)
	{
		if (Density[Idx] > LocalIsoLevel)
		{
			CubeIdx |= (1 << Idx);
		}
	}

	if (CubeIdx == 0 || CubeIdx == 255)
		return;

	for (int32 Idx = 0; TriangleTable[CubeIdx][Idx] != -1; Idx += 3)
	{
		FIntVector NewTriangle{};
		for (int32 TriangleIdx = 0; TriangleIdx < 3; ++TriangleIdx)
		{
			int32 EdgeIdx = TriangleTable[CubeIdx][Idx + TriangleIdx];

			int32 E1 = EdgeIndexTable[EdgeIdx][0];
			int32 E2 = EdgeIndexTable[EdgeIdx][1];

			// 버텍스의 위치를 보간해서 너무 각지지 않고 부드러워 보이게 함
			FVector P1 = GetVirtualPosition(Pos[E1], LocalLODLevel, NeighborMask, LocalVoxelSize, LocalGridSize);
			FVector P2 = GetVirtualPosition(Pos[E2], LocalLODLevel, NeighborMask, LocalVoxelSize, LocalGridSize);
			FVector FinalPos = Interpolate(P1, Density[E1], P2, Density[E2], LocalIsoLevel);

			int32* CachedIdx = VertexCache.Find(FinalPos);
			if (CachedIdx != nullptr)
			{
				NewTriangle[TriangleIdx] = *CachedIdx;
			}
			else
			{
				int32 VertexIdx = MeshData.Vertices.Add(FinalPos);
				VertexCache.Add(FinalPos, VertexIdx);
				NewTriangle[TriangleIdx] = VertexIdx;
			}
		}

		MeshData.Triangles.Add(NewTriangle);
	}
}

void UVoxelGroundChunk::GenerateTransitionCell(int32 FaceIdx, int32 X, int32 Y, int32 Z, int32 LocalLODLevel, int32 Step, uint8 NeighborMask, FChunkMeshData& MeshData, TMap<FVector, int32>& VertexCache, const TArray<uint8>& LocalDensity, float LocalVoxelSize, int32 LocalGridSize, uint8 LocalIsoLevel)
{
	FVector Pos[13]{};
	uint8 Density[13]{};
	int32 CaseCode = 0;

	int32 HalfStep = Step / 2;
	for (int32 Idx = 0; Idx < 9; ++Idx)
	{
		int32 U = TransitionCornerTable[Idx][0] * HalfStep;
		int32 V = TransitionCornerTable[Idx][1] * HalfStep;

		FIntVector SwizzledPos = GetSwizzledPos(FaceIdx, U, V, Step);
		checkf(SwizzledPos.X >= 0, TEXT("TransitionCell Wrong Swizzled Pos"));

		FIntVector P = FIntVector(X, Y, Z) + SwizzledPos;
		Pos[Idx] = FVector(P.X, P.Y, P.Z) * LocalVoxelSize;
		Density[Idx] = LocalDensity[GetIndex(P.X, P.Y, P.Z, LocalGridSize)];
	}

	Pos[9] = GetVirtualPosition(Pos[0], LocalLODLevel, NeighborMask, LocalVoxelSize, LocalGridSize);
	Pos[10] = GetVirtualPosition(Pos[2], LocalLODLevel, NeighborMask, LocalVoxelSize, LocalGridSize);
	Pos[11] = GetVirtualPosition(Pos[6], LocalLODLevel, NeighborMask, LocalVoxelSize, LocalGridSize);
	Pos[12] = GetVirtualPosition(Pos[8], LocalLODLevel, NeighborMask, LocalVoxelSize, LocalGridSize);
	Density[9] = Density[0];
	Density[10] = Density[2];
	Density[11] = Density[6];
	Density[12] = Density[8];

	static const uint8 CaseTable[9] = { 0, 1, 2, 5, 8, 7, 6, 3, 4 };
	for (int32 i = 0; i < 9; ++i)
	{
		if (Density[CaseTable[i]] > LocalIsoLevel)
		{
			CaseCode |= (1 << i);
		}
	}

	if (CaseCode == 0 || CaseCode == 511)
		return;

	uint8 RawClassIndex = TransitionCellClassTable[CaseCode];
	bool bFlipped = (RawClassIndex & 0x80) != 0;
	uint8 ClassIndex = RawClassIndex & 0x7F;
	const FTransitionCellData& CellData = TransitionCellDataTable[ClassIndex];
	int32 TriangleCount = CellData.GetTriangleCount();

	for (int32 i = 0; i < TriangleCount; ++i)
	{
		FIntVector NewTriangle{};
		for (int32 TriangleIdx = 0; TriangleIdx < 3; ++TriangleIdx)
		{
			int32 TableIdx = bFlipped ? (i * 3 + (2 - TriangleIdx)) : (i * 3 + TriangleIdx);
			int32 EdgeIdx = CellData.VertexIndex[TableIdx];
			uint16 EdgeData = TransitionVertexData[CaseCode][EdgeIdx];
			int32 Edge0 = (EdgeData >> 4) & 0x0F;
			int32 Edge1 = EdgeData & 0x0F;
			FVector FinalPos = Interpolate(Pos[Edge0], Density[Edge0], Pos[Edge1], Density[Edge1], LocalIsoLevel);

			int32* CachedIdx = VertexCache.Find(FinalPos);
			if (CachedIdx != nullptr)
			{
				NewTriangle[TriangleIdx] = *CachedIdx;
			}
			else
			{
				int32 VertexIdx = MeshData.Vertices.Add(FinalPos);
				VertexCache.Add(FinalPos, VertexIdx);
				NewTriangle[TriangleIdx] = VertexIdx;
			}
		}

		MeshData.Triangles.Add(NewTriangle);
	}
}

FVector UVoxelGroundChunk::GetVirtualPosition(FVector PrimaryPos, int32 LODIndex, uint8 NeighborMask, float LocalVoxelSize, int32 LocalGridSize)
{
	if (NeighborMask == 0)
		return PrimaryPos;

	// Eric-Lengyel 'Voxel-Based Terrain For Real-Time Virtual Simulations'의 식 4.2를 활용하여 수축할 정점의 위치 계산
	float K = (float)LODIndex;
	float S = (float)LocalGridSize / (1 << LODIndex);
	float CellSize = (float)(1 << LODIndex) * LocalVoxelSize;
	float W = FMath::Pow(2.0f, K - 2.0f) * LocalVoxelSize;
	float MaxPos = (float)(LocalGridSize) * LocalVoxelSize;;

	FVector Offset(0, 0, 0);
	const float Epsilon = 0.001f;

	bool bXNeg = (PrimaryPos.X < Epsilon);
	bool bXPos = (PrimaryPos.X > MaxPos - Epsilon);
	bool bYNeg = (PrimaryPos.Y < Epsilon);
	bool bYPos = (PrimaryPos.Y > MaxPos - Epsilon);
	bool bZNeg = (PrimaryPos.Z < Epsilon);
	bool bZPos = (PrimaryPos.Z > MaxPos - Epsilon);

	// X축 수축: -X 방향, +X 방향
	if ((bYNeg == false || (NeighborMask & (1 << 2))) && (bYPos == false || (NeighborMask & (1 << 3))) &&
		(bZNeg == false || (NeighborMask & (1 << 4))) && (bZPos == false || (NeighborMask & (1 << 5))))
	{
		if ((NeighborMask & (1 << 0)) && bXNeg)
		{
			Offset.X = (1.0f - (PrimaryPos.X / CellSize)) * W;
		}
		else if ((NeighborMask & (1 << 1)) && bXPos)
		{
			Offset.X = ((S - 1.0f) - (PrimaryPos.X / CellSize)) * W;
		}
	}

	// Y축 수축: -Y 방향, +Y 방향
	if ((bXNeg == false || (NeighborMask & (1 << 0))) && (bXPos == false || (NeighborMask & (1 << 1))) &&
		(bZNeg == false || (NeighborMask & (1 << 4))) && (bZPos == false || (NeighborMask & (1 << 5))))
	{
		if ((NeighborMask & (1 << 2)) && bYNeg)
		{
			Offset.Y = (1.0f - (PrimaryPos.Y / CellSize)) * W;
		}
		else if ((NeighborMask & (1 << 3)) && bYPos)
		{
			Offset.Y = ((S - 1.0f) - (PrimaryPos.Y / CellSize)) * W;
		}
	}

	// Z축 수축: -Z 방향, +Z 방향
	if ((bXNeg == false || (NeighborMask & (1 << 0))) && (bXPos == false || (NeighborMask & (1 << 1))) &&
		(bYNeg == false || (NeighborMask & (1 << 2))) && (bYPos == false || (NeighborMask & (1 << 3))))
	{
		if ((NeighborMask & (1 << 4)) && bZNeg)
		{
			Offset.Z = (1.0f - (PrimaryPos.Z / CellSize)) * W;
		}
		else if ((NeighborMask & (1 << 5)) && bZPos)
		{
			Offset.Z = ((S - 1.0f) - (PrimaryPos.Z / CellSize)) * W;
		}
	}

	return PrimaryPos + Offset;
}

FIntVector UVoxelGroundChunk::GetSwizzledPos(int32 FaceIdx, int32 U, int32 V, int32 Step)
{
	switch (FaceIdx)
	{
		case 0: return FIntVector(0, V, U);
		case 1: return FIntVector(Step, U, V);
		case 2: return FIntVector(U, 0, V);
		case 3: return FIntVector(V, Step, U);
		case 4: return FIntVector(V, U, 0);
		case 5: return FIntVector(U, V, Step);
	}
	return FIntVector(-1, -1, -1);
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
