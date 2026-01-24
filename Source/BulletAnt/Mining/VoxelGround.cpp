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
	FIntVector ChunkCoord = GetChunkCoord(ChunkIdx);
	FVector ChunkOffset = GetChunkOffset(ChunkCoord.X, ChunkCoord.Y, ChunkCoord.Z);

	int32 ChunkPoints = ChunkGridSize + 1;
	ChunkDatas[ChunkIdx].ChunkState = EChunkState::Ground;
	ChunkDatas[ChunkIdx].DensityValues.SetNumUninitialized(ChunkPoints * ChunkPoints * ChunkPoints);

	FVector GroundRange = FVector(GroundSize.X * 0.5f, GroundSize.Y * 0.5f, GroundSize.Z);
	for (int32 Z = 0; Z < ChunkPoints; ++Z)
	{
		for (int32 Y = 0; Y < ChunkPoints; ++Y)
		{
			for (int32 X = 0; X < ChunkPoints; ++X)
			{
				FVector WorldPos = ChunkOffset + FVector(X, Y, Z) * ChunkVoxelSize;
				bool bGround =
					WorldPos.X >= -GroundRange.X * 0.9f && WorldPos.X <= GroundRange.X * 0.9f &&
					WorldPos.Y >= -GroundRange.Y * 0.9f && WorldPos.Y <= GroundRange.Y * 0.9f &&
					WorldPos.Z >= -GroundRange.Z * 0.9f && WorldPos.Z < -400.0f;

				int32 PointIdx = Z * ChunkPoints * ChunkPoints + Y * ChunkPoints + X;
				ChunkDatas[ChunkIdx].DensityValues[PointIdx] = bGround == true ? 255 : 0;
			}
		}
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
	if (ChunkDatas[ChunkIdx].DensityValues.IsEmpty() == true)
		return;

	FIntVector Coord = GetChunkCoord(ChunkIdx);
	FVector RelativeLocation = GetChunkOffset(Coord.X, Coord.Y, Coord.Z);

	UVoxelGroundChunk* NewChunk = NewObject<UVoxelGroundChunk>(this);

	static UMaterial* DefaultMat = Cast<UMaterial>(StaticLoadObject(UMaterial::StaticClass(), nullptr, TEXT("/Engine/EngineMaterials/DefaultMaterial")));
	NewChunk->SetMaterial(0, DefaultMat);

	NewChunk->RegisterComponent();
	NewChunk->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
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


