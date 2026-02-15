#include "Mining/VoxelGroundChunk.h"
#include "Mining/MarchingCubeTable.h"
#include "Mining/VoxelGround.h"
#include "DynamicMesh/MeshNormals.h"
#include "DynamicMesh/DynamicMesh3.h"
#include "Kismet/KismetSystemLibrary.h"

int32 UVoxelGroundChunk::MaxLODLevel = 2;

void UVoxelGroundChunk::InitializeChunk(int32 InGridSize, float InVoxelSize, uint8 InIsoLevel)
{
	GridSize = InGridSize;
	VoxelSize = InVoxelSize;
	IsoLevel = InIsoLevel;
}

void UVoxelGroundChunk::CalculateMeshDataAsync(int32 UpdateID, AVoxelGround* VoxelGround, const TArray<uint8>& DensityValues, const TArray<EVoxelType> VoxelTypes, const FNeighborLOD& NeighborLOD, int32 LODLevel)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(CalculateMeshDataAsync);

	if (DensityValues.IsEmpty() == true)
		return;

	if (LODLevel >= 0)
	{
		CurrentLODLevel = LODLevel;
	}

	TArray<uint8> LocalDensity = DensityValues;
	TArray<EVoxelType> LocalVoxelTypes = VoxelTypes;
	TWeakObjectPtr<UVoxelGroundChunk> WeakThis(this);
	TWeakObjectPtr<AVoxelGround> WeakGround(VoxelGround);

	// 쓰레드 안전을 위해 멤버변수도 복사해서 전달
	// []안에 넣으면 값 복사되는 줄 알았는데 컴파일러가 this포인터를 캡쳐해서 멤버변수에 접근하게 된다고 함
	const int32 LocalLODLevel = CurrentLODLevel;
	const float LocalVoxelSize = VoxelSize;
	const int32 LocalGridSize = GridSize;
	const uint8 LocalIsoLevel = IsoLevel;
	const int32 LocalChunkIdx = ChunkIdxV;

	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask,
		[UpdateID, LocalChunkIdx, WeakThis, WeakGround, LocalDensity = MoveTemp(LocalDensity), LocalVoxelTypes = MoveTemp(LocalVoxelTypes), NeighborLOD, LocalLODLevel, LocalVoxelSize, LocalGridSize, LocalIsoLevel]()
		{
			FChunkMeshData MeshData;
			TMap<uint64, int32> VertexCache;

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

			FVoxelGenerationContext Context(LocalDensity, LocalVoxelTypes, LocalLODLevel, Step, LocalVoxelSize, LocalGridSize, LocalIsoLevel, NeighborMask, MeshData, VertexCache);

			auto CallGenerateTransition = [&](int32 X, int32 Y, int32 Z, int32 FaceIdx)
				{
					if (NeighborMask & (1 << FaceIdx))
					{
						GenerateTransitionCell(FaceIdx, X, Y, Z, Context);
					}
				};

			// 내부 RegularCell
			for (int32 Z = 0; Z <= MaxSize; Z += Step)
			{
				for (int32 Y = 0; Y <= MaxSize; Y += Step)
				{
					for (int32 X = 0; X <= MaxSize; X += Step)
					{
						GenerateRegularCell(X, Y, Z, Context);

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

			if (WeakGround.IsValid() == true)
			{
				FChunkUpdateResult UpdateResult;
				UpdateResult.UpdateID = UpdateID;
				UpdateResult.TargetChunkIdx = LocalChunkIdx;
				UpdateResult.MeshData = MoveTemp(MeshData);

				WeakGround->EnqueueChunkUpdateResult(MoveTemp(UpdateResult));
			}
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
	return 1 << LODLevel;
}

uint32 UVoxelGroundChunk::GetVertexInfoForKey(int32 X, int32 Y, int32 Z, uint8 ShrinkDir)
{
	// 청크의 한 변에 현재 16 ~ 32개씩 정점 저장 중
	// 최대 128개를 넘지 않을 거 같아서, (X/Y/Z 8bit씩 + ShrinkMask 8bit = 32bit)로 VertexKey 만듬
	// X, Y, Z 자료형은 일단 signed 상태로 써서 7bit까지만 쓰고(유지하고), 나중에 부족하면 바꿀 예정 (청크는 정점을 너무 많이 가지면 비효율적이어서 안 커질 듯)
	return ((uint32)X << 20) | ((uint32)Y << 13) | ((uint32)Z << 6) | ((uint32)ShrinkDir);
}

uint64 UVoxelGroundChunk::GetVertexKey(uint32 VertexInfo0, uint32 VertexInfo1)
{
	return ((uint64)VertexInfo0 << 32) | ((uint64)VertexInfo1);
}

void UVoxelGroundChunk::GenerateRegularCell(int32 X, int32 Y, int32 Z, const FVoxelGenerationContext& Context)
{
	FVector Pos[8]{};
	uint8 Density[8]{};
	EVoxelType VoxelTypes[8]{};
	uint32 VertexInfo[8]{};
	for (int32 Idx = 0; Idx < 8; ++Idx)
	{
		FIntVector Corner(X + CornerTable[Idx][0] * Context.Step, Y + CornerTable[Idx][1] * Context.Step, Z + CornerTable[Idx][2] * Context.Step);
		FVector LocalPos = FVector(Corner.X, Corner.Y, Corner.Z) * Context.VoxelSize;
		int32 PointIdx = GetIndex(Corner.X, Corner.Y, Corner.Z, Context.GridSize);
		Density[Idx] = Context.Density[PointIdx];
		VoxelTypes[Idx] = Context.VoxelTypes[PointIdx];
		uint8 ShrinkMask = GetAdjustedPosition(LocalPos, Pos[Idx], Context.LODLevel, Context.NeighborMask, Context.VoxelSize, Context.GridSize);
		VertexInfo[Idx] = GetVertexInfoForKey(Corner.X, Corner.Y, Corner.Z, ShrinkMask);
	}

	int32 CubeIdx = 0;
	for (int32 Idx = 0; Idx < 8; ++Idx)
	{
		if (Density[Idx] > Context.IsoLevel)
		{
			CubeIdx |= (1 << Idx);
		}
	}

	if (CubeIdx == 0 || CubeIdx == 255)
		return;

	for (int32 Idx = 0; TriangleTable[CubeIdx][Idx] != -1; Idx += 3)
	{
		FIntVector NewTriangle{};
		EVoxelType TriangleType = EVoxelType::None;
		for (int32 TriangleIdx = 0; TriangleIdx < 3; ++TriangleIdx)
		{
			int32 EdgeIdx = TriangleTable[CubeIdx][Idx + TriangleIdx];

			int32 E1 = EdgeIndexTable[EdgeIdx][0];
			int32 E2 = EdgeIndexTable[EdgeIdx][1];

			int32 GroundVertex = (Density[E1] > Context.IsoLevel) ? E1 : E2;

			if (TriangleType == EVoxelType::None)
			{
				TriangleType = VoxelTypes[GroundVertex];
			}
			else if (TriangleType != VoxelTypes[GroundVertex])
			{
				TriangleType = EVoxelType::NormalRock;
			}

			uint64 VertexKey = GetVertexKey(VertexInfo[E1], VertexInfo[E2]);

			int32* CachedIdx = Context.OutVertexCache.Find(VertexKey);
			if (CachedIdx != nullptr)
			{
				NewTriangle[TriangleIdx] = *CachedIdx;
			}
			else
			{
				// 버텍스의 위치를 보간해서 너무 각지지 않고 부드러워 보이게 함e);
				FVector FinalPos = Interpolate(Pos[E1], Density[E1], Pos[E2], Density[E2], Context.IsoLevel);

				int32 VertexIdx = Context.OutMeshData.Vertices.Add(FinalPos);
				Context.OutVertexCache.Add(VertexKey, VertexIdx);
				NewTriangle[TriangleIdx] = VertexIdx;
			}
		}

		Context.OutMeshData.Triangles.Add(NewTriangle);
		Context.OutMeshData.MaterialIDs.Add((uint8)TriangleType);
	}
}

void UVoxelGroundChunk::GenerateTransitionCell(int32 FaceIdx, int32 X, int32 Y, int32 Z, const FVoxelGenerationContext& Context)
{
	FIntVector VIdx[9]{};
	FVector Pos[13]{};
	uint8 Density[13]{};
	EVoxelType VoxelTypes[13]{};
	uint32 VertexInfo[13]{};
	int32 CaseCode = 0;

	int32 HalfStep = Context.Step / 2;
	for (int32 Idx = 0; Idx < 9; ++Idx)
	{
		int32 U = TransitionCornerTable[Idx][0] * HalfStep;
		int32 V = TransitionCornerTable[Idx][1] * HalfStep;

		FIntVector SwizzledPos = GetSwizzledPos(FaceIdx, U, V, Context.Step);
		checkf(SwizzledPos.X >= 0, TEXT("TransitionCell Wrong Swizzled Pos"));

		VIdx[Idx] = FIntVector(X, Y, Z) + SwizzledPos;
		Pos[Idx] = FVector(VIdx[Idx].X, VIdx[Idx].Y, VIdx[Idx].Z) * Context.VoxelSize;
		int32 PointIdx = GetIndex(VIdx[Idx].X, VIdx[Idx].Y, VIdx[Idx].Z, Context.GridSize);
		Density[Idx] = Context.Density[PointIdx];
		VoxelTypes[Idx] = Context.VoxelTypes[PointIdx];
		VertexInfo[Idx] = GetVertexInfoForKey(VIdx[Idx].X, VIdx[Idx].Y, VIdx[Idx].Z, 0);
	}

	auto SetVertexInfoTransition =
		[&](int32 Idx, int32 IdxSource)
		{
			uint8 ShrinkMask = GetAdjustedPosition(Pos[IdxSource], Pos[Idx], Context.LODLevel, Context.NeighborMask, Context.VoxelSize, Context.GridSize);
			Density[Idx] = Density[IdxSource];
			VoxelTypes[Idx] = VoxelTypes[IdxSource];
			VertexInfo[Idx] = GetVertexInfoForKey(VIdx[IdxSource].X, VIdx[IdxSource].Y, VIdx[IdxSource].Z, ShrinkMask);
		};

	SetVertexInfoTransition(9, 0);
	SetVertexInfoTransition(10, 2);
	SetVertexInfoTransition(11, 6);
	SetVertexInfoTransition(12, 8);

	static const uint8 CaseTable[9] = { 0, 1, 2, 5, 8, 7, 6, 3, 4 };
	for (int32 i = 0; i < 9; ++i)
	{
		if (Density[CaseTable[i]] > Context.IsoLevel)
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
		EVoxelType TriangleType = EVoxelType::None;
		for (int32 TriangleIdx = 0; TriangleIdx < 3; ++TriangleIdx)
		{
			int32 TableIdx = bFlipped ? (i * 3 + (2 - TriangleIdx)) : (i * 3 + TriangleIdx);
			int32 EdgeIdx = CellData.VertexIndex[TableIdx];
			uint16 EdgeData = TransitionVertexData[CaseCode][EdgeIdx];
			int32 Edge0 = (EdgeData >> 4) & 0x0F;
			int32 Edge1 = EdgeData & 0x0F;

			int32 GroundVertex = (Density[Edge0] > Context.IsoLevel) ? Edge0 : Edge1;

			//TriangleType = (TriangleType == EVoxelType::NormalRock || VoxelTypes[GroundVertex] < TriangleType) ? VoxelTypes[GroundVertex] : TriangleType;
			if (TriangleType == EVoxelType::None)
			{
				TriangleType = VoxelTypes[GroundVertex];
			}
			else if (TriangleType != VoxelTypes[GroundVertex])
			{
				TriangleType = EVoxelType::NormalRock;
			}

			uint64 VertexKey = GetVertexKey(VertexInfo[Edge0], VertexInfo[Edge1]);

			int32* CachedIdx = Context.OutVertexCache.Find(VertexKey);
			if (CachedIdx != nullptr)
			{
				NewTriangle[TriangleIdx] = *CachedIdx;
			}
			else
			{
				FVector FinalPos = Interpolate(Pos[Edge0], Density[Edge0], Pos[Edge1], Density[Edge1], Context.IsoLevel);

				int32 VertexIdx = Context.OutMeshData.Vertices.Add(FinalPos);
				Context.OutVertexCache.Add(VertexKey, VertexIdx);
				NewTriangle[TriangleIdx] = VertexIdx;
			}
		}

		Context.OutMeshData.Triangles.Add(NewTriangle);
		Context.OutMeshData.MaterialIDs.Add((uint8)TriangleType);
	}
}

uint8 UVoxelGroundChunk:: GetAdjustedPosition(const FVector& PrimaryPos, FVector& OutFinalPos, int32 LODIndex, uint8 NeighborMask, float LocalVoxelSize, int32 LocalGridSize)
{
	if (NeighborMask == 0)
	{
		OutFinalPos = PrimaryPos;
		return 0;
	}

	uint8 ShrinkMask = 0;

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
			ShrinkMask |= (1 << 0);
			Offset.X = (1.0f - (PrimaryPos.X / CellSize)) * W;
		}
		else if ((NeighborMask & (1 << 1)) && bXPos)
		{

			ShrinkMask |= (1 << 1);
			Offset.X = ((S - 1.0f) - (PrimaryPos.X / CellSize)) * W;
		}
	}

	// Y축 수축: -Y 방향, +Y 방향
	if ((bXNeg == false || (NeighborMask & (1 << 0))) && (bXPos == false || (NeighborMask & (1 << 1))) &&
		(bZNeg == false || (NeighborMask & (1 << 4))) && (bZPos == false || (NeighborMask & (1 << 5))))
	{
		if ((NeighborMask & (1 << 2)) && bYNeg)
		{
			ShrinkMask |= (1 << 2);
			Offset.Y = (1.0f - (PrimaryPos.Y / CellSize)) * W;
		}
		else if ((NeighborMask & (1 << 3)) && bYPos)
		{
			ShrinkMask |= (1 << 3);
			Offset.Y = ((S - 1.0f) - (PrimaryPos.Y / CellSize)) * W;
		}
	}

	// Z축 수축: -Z 방향, +Z 방향
	if ((bXNeg == false || (NeighborMask & (1 << 0))) && (bXPos == false || (NeighborMask & (1 << 1))) &&
		(bYNeg == false || (NeighborMask & (1 << 2))) && (bYPos == false || (NeighborMask & (1 << 3))))
	{
		if ((NeighborMask & (1 << 4)) && bZNeg)
		{
			ShrinkMask |= (1 << 4);
			Offset.Z = (1.0f - (PrimaryPos.Z / CellSize)) * W;
		}
		else if ((NeighborMask & (1 << 5)) && bZPos)
		{
			ShrinkMask |= (1 << 5);
			Offset.Z = ((S - 1.0f) - (PrimaryPos.Z / CellSize)) * W;
		}
	}

	OutFinalPos = PrimaryPos + Offset;
	return ShrinkMask;
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

void UVoxelGroundChunk::UpdateChunk(int32 UpdateID, const FChunkMeshData& MeshData, bool bUpdatePhysics)
{
	LastUpdateID = UpdateID;
	UDynamicMesh* Mesh = GetDynamicMesh();
	if (IsValid(Mesh) == false)
		return;

	FDynamicMesh3& DynamicMesh3 = Mesh->GetMeshRef();
	DynamicMesh3.Clear();

	DynamicMesh3.EnableAttributes();
	DynamicMesh3.Attributes()->EnableMaterialID();

	for (const FVector& Vertex : MeshData.Vertices)
	{
		DynamicMesh3.AppendVertex(Vertex);
	}

	for (int32 Idx = 0; Idx < MeshData.Triangles.Num(); ++Idx)
	{
		const FIntVector& Triangle = MeshData.Triangles[Idx];
		int32 TriangleID = DynamicMesh3.AppendTriangle(Triangle.X, Triangle.Y, Triangle.Z);
		DynamicMesh3.Attributes()->GetMaterialID()->SetValue(TriangleID, (int32)MeshData.MaterialIDs[Idx]);
	}

	UE::Geometry::FMeshNormals::QuickComputeVertexNormals(DynamicMesh3);

	// 렌더링 업데이트
	NotifyMeshUpdated();

	UpdateCollision(false);
	//if (bUpdatePhysics)
	//{
	//	// 충돌체 업데이트
	//	UpdateCollision(false);

	//	SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	//	SetGenerateOverlapEvents(true);
	//}
	//else
	//{
	//	SetCollisionEnabled(ECollisionEnabled::NoCollision);
	//	SetGenerateOverlapEvents(false);
	//}

	UpdateBounds();
}

void UVoxelGroundChunk::SetMaxLODLevel(int32 InMaxLODLevel)
{
	MaxLODLevel = InMaxLODLevel;
}
