#include "Mining/VoxelGround.h"
#include "Mining/VoxelGroundChunk.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"

// -X, +X, -Y, +Y, -Z, +Z
const FIntVector AVoxelGround::NeighborOffsets[6] = { { -1, 0, 0 }, { 1, 0, 0 }, { 0, -1, 0 }, { 0, 1, 0 }, { 0, 0, -1 }, { 0, 0, 1 } };

AVoxelGround::AVoxelGround()
{
	PrimaryActorTick.bCanEverTick = true;

}

void AVoxelGround::BeginPlay()
{
	Super::BeginPlay();

	UVoxelGroundChunk::SetMaxLODLevel(LODDistance.Num());
	
	InitializeGround();

	GetWorldTimerManager().SetTimer(
		UpdateChunkLODTimerHandle,
		this,
		&ThisClass::UpdateChunkLODs,
		0.5f,
		true);

	GetWorldTimerManager().SetTimer(
		UpdateDirtyChunkTimerHandle,
		this,
		&ThisClass::UpdateDirtyChunks,
		1.0f,
		true,
		0.25f);
}

void AVoxelGround::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearAllTimersForObject(this);

	Super::EndPlay(EndPlayReason);
}

void AVoxelGround::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	int32 UpdateCount = 0;
	while (ChunkUpdateResultQueue.IsEmpty() == false && UpdateCount < MaxUpdatePerFrame)
	{
		FChunkUpdateResult Result;
		ChunkUpdateResultQueue.Dequeue(Result);

		UVoxelGroundChunk* TargetChunk = Chunks[Result.TargetChunkIdx];
		if (IsValid(TargetChunk) == false || Result.UpdateID <= TargetChunk->GetLastUpdateID())
			continue;

		bool bIsNearPlayer = (GetChunkLODLevel(Result.TargetChunkIdx) == 0);
		TargetChunk->UpdateChunk(Result.UpdateID, Result.MeshData, bIsNearPlayer);

		++UpdateCount;
	}
}

void AVoxelGround::DigGround(const FVector& WorldLocation, float Radius)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(DigGround);
	FVector RelativeLocation = WorldLocation - GetActorLocation();
	float ChunkSize = ChunkGridSize * ChunkVoxelSize;

	int32 MinX = FMath::FloorToInt((RelativeLocation.X - Radius) / ChunkSize);
	int32 MaxX = FMath::FloorToInt((RelativeLocation.X + Radius) / ChunkSize);
	int32 MinY = FMath::FloorToInt((RelativeLocation.Y - Radius) / ChunkSize);
	int32 MaxY = FMath::FloorToInt((RelativeLocation.Y + Radius) / ChunkSize);
	int32 MinZ = FMath::FloorToInt((RelativeLocation.Z - Radius) / ChunkSize);
	int32 MaxZ = FMath::FloorToInt((RelativeLocation.Z + Radius) / ChunkSize);

	TSet<int32> UpdatedChunks;
	for (int32 Z = MinZ; Z <= MaxZ; ++Z)
	{
		for (int32 Y = MinY; Y <= MaxY; ++Y)
		{
			for (int32 X = MinX; X <= MaxX; ++X)
			{
				int32 ChunkIdx = GetChunkIndex(X, Y, Z);
				if (ChunkDatas.IsValidIndex(ChunkIdx) == false)
					continue;

				if (ChunkDatas[ChunkIdx].DensityValues.IsEmpty() == true)
				{
					// 모두 땅이라서 DensityValues를 비워뒀다면 초기화 후 Dig
					if (ChunkDatas[ChunkIdx].ChunkState == EChunkState::Ground)
					{
						int32 ChunkPoints = ChunkGridSize + 1;
						ChunkDatas[ChunkIdx].DensityValues.Init(255, ChunkPoints * ChunkPoints * ChunkPoints);
					}
					else
					{
						continue;
					}
				}

				FVector ChunkOffset = FVector(X, Y, Z) * ChunkSize;
				DigGround(ChunkIdx, ChunkOffset, WorldLocation, Radius);
			}
		}
	}
}

void AVoxelGround::DigGround(int32 ChunkIdx, const FVector& ChunkOffset, const FVector& WorldLocation, float Radius)
{
	FVector RelativeLocation = WorldLocation - GetActorLocation();
	float ChunkSize = ChunkGridSize * ChunkVoxelSize;
	bool bChunkModified = false;
	int32 ChunkPoints = ChunkGridSize + 1;
	float RadiusSquared = Radius * Radius;

	for (int32 Z = 0; Z < ChunkPoints; ++Z)
	{
		for (int32 Y = 0; Y < ChunkPoints; ++Y)
		{
			for (int32 X = 0; X < ChunkPoints; ++X)
			{
				FVector PointWorldPos = ChunkOffset + FVector(X, Y, Z) * ChunkVoxelSize;
				float DistSquared = FVector::DistSquared(RelativeLocation, PointWorldPos);

				if (DistSquared <= RadiusSquared)
				{
					int32 PointIdx = Z * ChunkPoints * ChunkPoints + Y * ChunkPoints + X;

					// Dig 중심부에 가까울 수록 값을 많이 깎아서 부드러운 벽이 형성됨 / 약간 답답함
					float Dist = FMath::Sqrt(DistSquared);
					float DistRatio = FMath::Clamp(Dist / Radius, 0.0f, 1.0f);
					float SmoothAlpha = 1 - (DistRatio * DistRatio * (3.0f - 2.0f * DistRatio));
					uint8 DeltaDensity = FMath::RoundToInt(SmoothAlpha * 255);
					
					uint8 CurrentDensity = ChunkDatas[ChunkIdx].DensityValues[PointIdx];
					uint8 TargetDensity = FMath::Max(0, CurrentDensity - DeltaDensity);
					if (TargetDensity < CurrentDensity)
					{
						ChunkDatas[ChunkIdx].DensityValues[PointIdx] = TargetDensity;
						bChunkModified = true;
					}

					// 한번에 0으로 값을 바꿔서 벽이 단번에 깎이는 느낌이 잘 듬 / 벽이 너무 각져 있음 
					/*if (ChunkDatas[ChunkIdx].DensityValues[PointIdx] > 127)
					{
						ChunkDatas[ChunkIdx].DensityValues[PointIdx] = 0;
						bChunkModified = true;
					}*/
				}
			}
		}
	}

	if (bChunkModified == true)
	{
		ChunkDatas[ChunkIdx].ChunkState = EChunkState::Complex;

		if (IsValid(Chunks[ChunkIdx]) == false)
		{
			SpawnChunk(ChunkIdx);
		}

		if (IsValid(Chunks[ChunkIdx]) == true)
		{
			UpdateChunkMeshImmediately(ChunkIdx, false);
		}
	}
}

void AVoxelGround::EnqueueChunkUpdateResult(FChunkUpdateResult&& Result)
{
	ChunkUpdateResultQueue.Enqueue(Forward<FChunkUpdateResult>(Result));
}

void AVoxelGround::InitializeGround()
{
	TRACE_CPUPROFILER_EVENT_SCOPE(InitializeGround);
	float ChunkSize = ChunkGridSize * ChunkVoxelSize;

	ChunkRangeMax.X = FMath::CeilToInt(GroundSize.X * 0.5f / ChunkSize) + 1;
	ChunkRangeMax.Y = FMath::CeilToInt(GroundSize.Y * 0.5f / ChunkSize) + 1;
	ChunkRangeMax.Z = -1;
	ChunkRangeMin.X = -ChunkRangeMax.X;
	ChunkRangeMin.Y = -ChunkRangeMax.Y;
	ChunkRangeMin.Z = -(FMath::CeilToInt(GroundSize.Z / ChunkSize) + 1);
	GridWidth = ChunkRangeMax - ChunkRangeMin + FIntVector(1, 1, 1);

	int ChunkCount = 
		(ChunkRangeMax.X - ChunkRangeMin.X + 1) * 
		(ChunkRangeMax.Y - ChunkRangeMin.Y + 1) *
		(ChunkRangeMax.Z - ChunkRangeMin.Z + 1);

	ChunkDatas.SetNum(ChunkCount);
	Chunks.SetNum(ChunkCount);
	ChunkMeshDirties.Init(false, ChunkCount);
	PriorityChunkMeshDirties.Init(false, ChunkCount);

	for (int32 Z = ChunkRangeMin.Z; Z <= ChunkRangeMax.Z; ++Z)
	{
		for (int32 Y = ChunkRangeMin.Y; Y <= ChunkRangeMax.Y; ++Y)
		{
			for (int32 X = ChunkRangeMin.X; X <= ChunkRangeMax.X; ++X)
			{
				int32 ChunkIdx = GetChunkIndex(X, Y, Z);
				InitializeChunkData(ChunkIdx);
			}
		}
	}

	for (int32 Z = ChunkRangeMin.Z; Z <= ChunkRangeMax.Z; ++Z)
	{
		for (int32 Y = ChunkRangeMin.Y; Y <= ChunkRangeMax.Y; ++Y)
		{
			for (int32 X = ChunkRangeMin.X; X <= ChunkRangeMax.X; ++X)
			{
				int32 ChunkIdx = GetChunkIndex(X, Y, Z);
				SpawnChunk(ChunkIdx);
				UpdateChunkMeshImmediately(ChunkIdx, false);
			}
		}
	}
}

void AVoxelGround::InitializeChunkData(int32 ChunkIdx)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(InitializeChunkData);

	FIntVector ChunkCoord = GetChunkCoord(ChunkIdx);
	FVector ChunkOffset = GetChunkOffset(ChunkCoord.X, ChunkCoord.Y, ChunkCoord.Z);

	int32 ChunkPoints = ChunkGridSize + 1;
	//ChunkDatas[ChunkIdx].DensityValues.SetNumUninitialized(ChunkPoints * ChunkPoints * ChunkPoints);

	TArray<uint8> TempDensities;
	TempDensities.SetNumUninitialized(ChunkPoints * ChunkPoints * ChunkPoints);

	ChunkDatas[ChunkIdx].LODLevel = LODDistance.Num();
	ChunkDatas[ChunkIdx].ChunkState = EChunkState::Ground;

	// 쓰레드 안전을 위해 Atomic 사용
	std::atomic<bool> bAtomicHasAir(false);
	std::atomic<bool> bAtomicHasGround(false);

	const FVector GroundMinRange = FVector(-GroundSize.X * 0.45f, -GroundSize.Y * 0.45f, -GroundSize.Z);
	const FVector GroundMaxRange = FVector(GroundSize.X * 0.45f, GroundSize.Y * 0.45f, -400.0f);
	ParallelFor(ChunkPoints, [&](int32 Z)
		{
			bool bHasAir = false;
			bool bHasGround = false;

			for (int32 Y = 0; Y < ChunkPoints; ++Y)
			{
				for (int32 X = 0; X < ChunkPoints; ++X)
				{
					FVector WorldPos = ChunkOffset + FVector(X, Y, Z) * ChunkVoxelSize;

					uint8 FinalDensity = 0;

					bool bGround = 
						WorldPos.X >= GroundMinRange.X && WorldPos.X <= GroundMaxRange.X &&
						WorldPos.Y >= GroundMinRange.Y && WorldPos.Y <= GroundMaxRange.Y &&
						WorldPos.Z >= GroundMinRange.Z && WorldPos.Z <= GroundMaxRange.Z;

					// 땅이면 Cave, Tunnel 확률적 생성 및 Density 설정
					if (bGround == true)
					{
						float Density = 255.0f;

						float CaveValue = FMath::PerlinNoise3D(WorldPos * CaveScale);
						float CaveDensity = 127.5f + (CaveThreshold - CaveValue) * 255.0f;

						Density = FMath::Min(Density, CaveDensity);
						FinalDensity = (uint8)FMath::Clamp(FMath::RoundToInt(Density), 0, 255);
					}

					int32 PointIdx = Z * ChunkPoints * ChunkPoints + Y * ChunkPoints + X;
					TempDensities[PointIdx] = FinalDensity;

					if (FinalDensity > IsoLevel)
					{
						bHasGround = true;
					}
					else
					{
						bHasAir = true;
					}
				}
			}

			if (bHasAir == true)
			{
				bAtomicHasAir = true;
			}
			if (bHasGround == true)
			{
				bAtomicHasGround = true;
			}
		});

	bool bFinalHasAir = bAtomicHasAir.load();
	bool bFinalHasGround = bAtomicHasGround.load();

	// 모두 땅이거나 모두 공기이면(청크가 단일 상태라면) 데이터도 저장 X
	if (bFinalHasAir == true && bFinalHasGround == true)
	{
		ChunkDatas[ChunkIdx].ChunkState = EChunkState::Complex;
		ChunkDatas[ChunkIdx].DensityValues = MoveTemp(TempDensities);
	}
	else
	{
		ChunkDatas[ChunkIdx].ChunkState = bFinalHasGround == true ? EChunkState::Ground : EChunkState::Air;
		ChunkDatas[ChunkIdx].DensityValues.Empty(0);
	}
}

int32 AVoxelGround::GetChunkIndex(int32 X, int32 Y, int32 Z) const
{
	return (Z - ChunkRangeMin.Z) * GridWidth.X * GridWidth.Y + (Y - ChunkRangeMin.Y) * GridWidth.X + (X - ChunkRangeMin.X);
}

int32 AVoxelGround::GetChunkIndex(const FIntVector ChunkCoord) const
{
	return GetChunkIndex(ChunkCoord.X, ChunkCoord.Y, ChunkCoord.Z);
}

FIntVector AVoxelGround::GetChunkCoord(int32 ChunkIdx) const
{
	FIntVector Coord = FIntVector::ZeroValue;
	Coord.Z = ChunkIdx / (GridWidth.X * GridWidth.Y) + ChunkRangeMin.Z;
	ChunkIdx %= (GridWidth.X * GridWidth.Y);
	Coord.Y = ChunkIdx / GridWidth.X + ChunkRangeMin.Y;
	ChunkIdx %= GridWidth.X;
	Coord.X = ChunkIdx + ChunkRangeMin.X;
	return Coord;
}

FVector AVoxelGround::GetChunkOffset(int32 X, int32 Y, int32 Z) const
{
	float ChunkSize = ChunkGridSize * ChunkVoxelSize;
	return FVector(X, Y, Z) * ChunkSize;
}

int32 AVoxelGround::GetLODLevelByPlayer(const FIntVector& ChunkCoord, const FIntVector& PlayerChunkCoord, int32 CurrentLODLevel)
{
	int32 DistX = FMath::Abs(ChunkCoord.X - PlayerChunkCoord.X);
	int32 DistY = FMath::Abs(ChunkCoord.Y - PlayerChunkCoord.Y);
	int32 DistZ = FMath::Abs(ChunkCoord.Z - PlayerChunkCoord.Z);
	int32 Distance = DistX * DistX + DistY * DistY + DistZ * DistZ;

	for (int32 Idx = 0; Idx < LODDistance.Num(); ++Idx)
	{
		int32 Threshold = LODDistance[Idx];
		if (CurrentLODLevel <= Idx)
		{
			Threshold += LODDistMargin;
		}

		if (Distance <= Threshold * Threshold)
			return Idx;
	}
	return LODDistance.Num();
}

int32 AVoxelGround::GetChunkLODLevel(const FIntVector& ChunkCoord)
{
	int32 ChunkIdx = GetChunkIndex(ChunkCoord.X, ChunkCoord.Y, ChunkCoord.Z);
	return GetChunkLODLevel(ChunkIdx);
}

int32 AVoxelGround::GetChunkLODLevel(int32 ChunkIdx)
{
	if (ChunkDatas.IsValidIndex(ChunkIdx) == false)
		return LODDistance.Num();

	return ChunkDatas[ChunkIdx].LODLevel;
}

FNeighborLOD AVoxelGround::GetNeighborLOD(const FIntVector& ChunkCoord)
{
	FNeighborLOD NeighborLOD{};

	for (int32 Idx = 0; Idx < 6; ++Idx)
	{
		NeighborLOD.LODs[Idx] = GetChunkLODLevel(ChunkCoord + NeighborOffsets[Idx]);
	}
	return NeighborLOD;
}

FNeighborLOD AVoxelGround::GetNeighborLOD(int32 ChunkIdx)
{
	return GetNeighborLOD(GetChunkCoord(ChunkIdx));
}

void AVoxelGround::AddChunkAndNeighbors(int32 ChunkIdx, TSet<int32>& ChunkIdxs)
{
	ChunkIdxs.Add(ChunkIdx);

	int32 ChunkLODLevel = GetChunkLODLevel(ChunkIdx);
	FIntVector ChunkCoord = GetChunkCoord(ChunkIdx);
	for (int32 Idx = 0; Idx < 6; ++Idx)
	{
		int32 NeighborIdx = GetChunkIndex(ChunkCoord + NeighborOffsets[Idx]);
		if (GetChunkLODLevel(NeighborIdx) > ChunkLODLevel)
		{
			ChunkIdxs.Add(NeighborIdx);
		}
	}
}

void AVoxelGround::UpdateChunkMesh(int32 ChunkIdx, bool bIncludeNeighbors)
{
	bIsDirty = true;
	ChunkMeshDirties[ChunkIdx] = true;
	
	if (bIncludeNeighbors == true)
	{
		FIntVector ChunkCoord = GetChunkCoord(ChunkIdx);

		for (int32 Idx = 0; Idx < 6; ++Idx)
		{
			int32 NeighborIdx = GetChunkIndex(ChunkCoord + NeighborOffsets[Idx]);
			if (ChunkMeshDirties.IsValidIndex(NeighborIdx) == false || ChunkMeshDirties[NeighborIdx] == true)
				continue;

			ChunkMeshDirties[NeighborIdx] = true;
		}
	}
}

void AVoxelGround::UpdateChunkMeshImmediately(int32 ChunkIdx, bool bIncludeNeighbors)
{
	if (bIsPriorityDirty == false)
	{
		bIsPriorityDirty = true;
		GetWorldTimerManager().SetTimerForNextTick(
			this,
			&ThisClass::UpdatePriorityDirtyChunks);
	}

	PriorityChunkMeshDirties[ChunkIdx] = true;

	if (bIncludeNeighbors == true)
	{
		FIntVector ChunkCoord = GetChunkCoord(ChunkIdx);

		for (int32 Idx = 0; Idx < 6; ++Idx)
		{
			int32 NeighborIdx = GetChunkIndex(ChunkCoord + NeighborOffsets[Idx]);
			if (PriorityChunkMeshDirties.IsValidIndex(NeighborIdx) == false || PriorityChunkMeshDirties[NeighborIdx] == true)
				continue;

			PriorityChunkMeshDirties[NeighborIdx] = true;
		}
	}
}

void AVoxelGround::UpdateDirtyChunks()
{
	if (bIsDirty == false)
		return;

	int32 Count = 0;
	bIsDirty = false;
	for (TBitArray<>::FConstIterator It(ChunkMeshDirties); It; ++It)
	{
		int32 ChunkIdx = It.GetIndex();
		if (ChunkMeshDirties[ChunkIdx] == true)
		{
			if (IsValid(Chunks[ChunkIdx]) == true)
			{
				Chunks[ChunkIdx]->CalculateMeshDataAsync(NextChunkUpdateID++, this, ChunkDatas[ChunkIdx].DensityValues, GetNeighborLOD(ChunkIdx), GetChunkLODLevel(ChunkIdx));
			}
			ChunkMeshDirties[ChunkIdx] = false;
			++Count;
		}
	}

	// UKismetSystemLibrary::PrintString(GetWorld(), *FString::FromInt(Count));
}

void AVoxelGround::UpdatePriorityDirtyChunks()
{
	if (bIsPriorityDirty == false)
		return;

	bIsPriorityDirty = false;
	for (TBitArray<>::FConstIterator It(PriorityChunkMeshDirties); It; ++It)
	{
		int32 ChunkIdx = It.GetIndex();
		if (PriorityChunkMeshDirties[ChunkIdx] == true)
		{
			if (IsValid(Chunks[ChunkIdx]) == true)
			{
				Chunks[ChunkIdx]->CalculateMeshDataAsync(NextChunkUpdateID++, this, ChunkDatas[ChunkIdx].DensityValues, GetNeighborLOD(ChunkIdx), GetChunkLODLevel(ChunkIdx));
			}
			PriorityChunkMeshDirties[ChunkIdx] = false;
		}
	}
}

void AVoxelGround::SpawnChunk(int32 ChunkIdx)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(SpawnChunk);

	if (ChunkDatas[ChunkIdx].ChunkState != EChunkState::Complex || ChunkDatas[ChunkIdx].DensityValues.IsEmpty() == true)
		return;

	FIntVector Coord = GetChunkCoord(ChunkIdx);
	FVector RelativeLocation = GetChunkOffset(Coord.X, Coord.Y, Coord.Z);

	UVoxelGroundChunk* NewChunk = NewObject<UVoxelGroundChunk>(this);

	static UMaterial* DefaultMat = Cast<UMaterial>(StaticLoadObject(UMaterial::StaticClass(), nullptr, TEXT("/Engine/EngineMaterials/DefaultMaterial")));
	NewChunk->SetMaterial(0, DefaultMat);

	NewChunk->RegisterComponent();
	NewChunk->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
	NewChunk->bUseAsyncCooking = true;
	NewChunk->bCastVolumetricTranslucentShadow = false;
	NewChunk->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	NewChunk->SetCollisionResponseToAllChannels(ECR_Ignore);
	NewChunk->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	NewChunk->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	NewChunk->SetComplexAsSimpleCollisionEnabled(true);
	NewChunk->SetRelativeLocation(RelativeLocation);
	AddInstanceComponent(NewChunk);
	Chunks[ChunkIdx] = NewChunk;

	NewChunk->InitializeChunk(ChunkGridSize, ChunkVoxelSize, IsoLevel);
	NewChunk->CoordV = Coord;
	NewChunk->ChunkIdxV = ChunkIdx;
}

FVector AVoxelGround::GetPlayerLocation()
{
	ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	if (!ensureMsgf(IsValid(PlayerCharacter), TEXT("VoxelGround => Cannot find PlayerCharacter")))
	{
		return FVector::ZeroVector;
	}

	return PlayerCharacter->GetActorLocation();
}

void AVoxelGround::CheckPlayerChunk(const FVector& PlayerLocation)
{
	float ChunkSize = ChunkGridSize * ChunkVoxelSize;
	FVector RelativePlayerLocation = PlayerLocation - GetActorLocation();

	// 중심 청크 (플레이어가 현재 위치한 청크)
	FIntVector CurrentPlayerChunkCoord = FIntVector(
		FMath::FloorToInt(RelativePlayerLocation.X / ChunkSize),
		FMath::FloorToInt(RelativePlayerLocation.Y / ChunkSize),
		FMath::FloorToInt(RelativePlayerLocation.Z / ChunkSize));

	if (CurrentPlayerChunkCoord != LastPlayerChunkCoord)
	{
		UpdateNearByChunks(CurrentPlayerChunkCoord);
		LastPlayerChunkCoord = CurrentPlayerChunkCoord;
	}
}

void AVoxelGround::UpdateNearByChunks(const FIntVector& PlayerChunkCoord)
{
	TSet<int32> CurrNearByChunkIdxs;
	TSet<int32> ChunkIdxsToCheck = LastNearByChunkIdxs;

	int32 MaxDistWithMargin = LODDistance.Last() + LODDistMargin;
	int32 MaxDistance = MaxDistWithMargin;
	FIntVector MinRange = PlayerChunkCoord - FIntVector(MaxDistance, MaxDistance, MaxDistance);
	FIntVector MaxRange = PlayerChunkCoord + FIntVector(MaxDistance, MaxDistance, MaxDistance);

	for (int32 Z = MinRange.Z; Z <= MaxRange.Z; ++Z)
	{
		for (int32 Y = MinRange.Y; Y <= MaxRange.Y; ++Y)
		{
			for (int32 X = MinRange.X; X <= MaxRange.X; ++X)
			{
				int32 ChunkIdx = GetChunkIndex(X, Y, Z);
				if (ChunkDatas.IsValidIndex(ChunkIdx) == false)
					continue;

				ChunkIdxsToCheck.Add(ChunkIdx);
			}
		}
	}

	for (int32 ChunkIdx : ChunkIdxsToCheck)
	{
		FIntVector ChunkCoord = GetChunkCoord(ChunkIdx);

		int32 OldLODLevel = ChunkDatas[ChunkIdx].LODLevel;
		int32 NewLODLevel = GetLODLevelByPlayer(ChunkCoord, PlayerChunkCoord, OldLODLevel);

		if (NewLODLevel != OldLODLevel)
		{
			ChunkDatas[ChunkIdx].LODLevel = NewLODLevel;
			UpdateChunkMesh(ChunkIdx, true);
		}

		if (NewLODLevel < LODDistance.Num())
		{
			CurrNearByChunkIdxs.Add(ChunkIdx);
		}
	}

	LastNearByChunkIdxs = MoveTemp(CurrNearByChunkIdxs);
}

void AVoxelGround::UpdateChunkLODs()
{
	const FVector& PlayerLocation = GetPlayerLocation();
	CheckPlayerChunk(PlayerLocation);
}


