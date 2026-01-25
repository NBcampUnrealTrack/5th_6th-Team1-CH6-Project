#include "Mining/VoxelGround.h"
#include "Mining/VoxelGroundChunk.h"

AVoxelGround::AVoxelGround()
{
	PrimaryActorTick.bCanEverTick = false;

}

void AVoxelGround::BeginPlay()
{
	Super::BeginPlay();
	
	InitializeGround();
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
				if (X < ChunkRangeMin.X || X > ChunkRangeMax.X || Y < ChunkRangeMin.Y || Y > ChunkRangeMax.Y || Z < ChunkRangeMin.Z || Z > ChunkRangeMax.Z)
					continue;

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
			Chunks[ChunkIdx]->UpdateMesh(ChunkDatas[ChunkIdx].DensityValues);
		}
	}
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

	for (int32 Z = ChunkRangeMin.Z; Z <= ChunkRangeMax.Z; ++Z)
	{
		for (int32 Y = ChunkRangeMin.Y; Y <= ChunkRangeMax.Y; ++Y)
		{
			for (int32 X = ChunkRangeMin.X; X <= ChunkRangeMax.X; ++X)
			{
				int32 ChunkIdx = GetChunkIndex(X, Y, Z);

				InitializeDensityPerChunk(ChunkIdx);
				SpawnChunk(ChunkIdx);
			}
		}
	}
}

void AVoxelGround::InitializeDensityPerChunk(int32 ChunkIdx)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(InitializeDensityPerChunk);

	FIntVector ChunkCoord = GetChunkCoord(ChunkIdx);
	FVector ChunkOffset = GetChunkOffset(ChunkCoord.X, ChunkCoord.Y, ChunkCoord.Z);

	int32 ChunkPoints = ChunkGridSize + 1;
	//ChunkDatas[ChunkIdx].DensityValues.SetNumUninitialized(ChunkPoints * ChunkPoints * ChunkPoints);

	TArray<uint8> TempDensities;
	TempDensities.SetNumUninitialized(ChunkPoints * ChunkPoints * ChunkPoints);

	ChunkDatas[ChunkIdx].ChunkState = EChunkState::Ground;

	// 쓰레드 안전을 위해 Atomic 사용
	std::atomic<bool> bAtomicHasAir(false);
	std::atomic<bool> bAtomicHasGround(false);

	FVector GroundRange = FVector(GroundSize.X * 0.5f, GroundSize.Y * 0.5f, GroundSize.Z);
	ParallelFor(ChunkPoints, [&](int32 Z)
		{
			bool bHasAir = false;
			bool bHasGround = false;

			for (int32 Y = 0; Y < ChunkPoints; ++Y)
			{
				for (int32 X = 0; X < ChunkPoints; ++X)
				{
					FVector WorldPos = ChunkOffset + FVector(X, Y, Z) * ChunkVoxelSize;
					bool bGround =
						WorldPos.X >= -GroundRange.X * 0.9f && WorldPos.X <= GroundRange.X * 0.9f &&
						WorldPos.Y >= -GroundRange.Y * 0.9f && WorldPos.Y <= GroundRange.Y * 0.9f &&
						WorldPos.Z >= -GroundRange.Z * 0.9f && WorldPos.Z < -400.0f;

					if (bGround == true)
					{
						bHasGround = true;
					}
					else
					{
						bHasAir = true;
					}

					int32 PointIdx = Z * ChunkPoints * ChunkPoints + Y * ChunkPoints + X;
					TempDensities[PointIdx] = bGround == true ? 255 : 0;
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
	NewChunk->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	NewChunk->SetCollisionResponseToAllChannels(ECR_Ignore);
	NewChunk->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	NewChunk->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	NewChunk->SetComplexAsSimpleCollisionEnabled(true);
	NewChunk->SetRelativeLocation(RelativeLocation);
	AddInstanceComponent(NewChunk);
	Chunks[ChunkIdx] = NewChunk;

	NewChunk->InitializeChunk(ChunkGridSize, ChunkVoxelSize, IsoLevel);
	NewChunk->UpdateMesh(ChunkDatas[ChunkIdx].DensityValues);
}


