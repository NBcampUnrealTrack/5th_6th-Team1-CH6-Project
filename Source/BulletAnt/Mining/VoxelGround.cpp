#include "Mining/VoxelGround.h"
#include "Mining/VoxelGroundChunk.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"

AVoxelGround::AVoxelGround()
{
	PrimaryActorTick.bCanEverTick = false;

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
		1.0f,
		true);
}

void AVoxelGround::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearAllTimersForObject(this);

	Super::EndPlay(EndPlayReason);
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
			Chunks[ChunkIdx]->UpdateMeshAsync(ChunkDatas[ChunkIdx].DensityValues, GetNeighborLOD(ChunkIdx));
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

					uint8 FinalDensity = 0;

					bool bGround =
						WorldPos.X >= -GroundRange.X * 0.9f && WorldPos.X <= GroundRange.X * 0.9f &&
						WorldPos.Y >= -GroundRange.Y * 0.9f && WorldPos.Y <= GroundRange.Y * 0.9f &&
						WorldPos.Z >= -GroundRange.Z * 0.9f && WorldPos.Z < -400.0f;

					// 땅이면 Cave, Tunnel 확률적 생성 및 Density 설정
					if (bGround == true)
					{
						float Density = 255.0f;

						float CaveValue = FMath::PerlinNoise3D(WorldPos * CaveScale);
						float CaveDensity = 127.5f + (CaveThreshold - CaveValue) * 255.0f;

						Density = FMath::Min(Density, CaveDensity);
						FinalDensity = uint8(FMath::Clamp(FMath::RoundToInt(Density), 0, 255));
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

int32 AVoxelGround::GetLODLevelByPlayer(const FIntVector& ChunkCoord, const FIntVector& PlayerChunkCoord)
{
	int32 DistX = FMath::Abs(ChunkCoord.X - PlayerChunkCoord.X);
	int32 DistY = FMath::Abs(ChunkCoord.Y - PlayerChunkCoord.Y);
	int32 DistZ = FMath::Abs(ChunkCoord.Z - PlayerChunkCoord.Z);
	int32 Distance = FMath::Max(DistX, FMath::Max(DistY, DistZ));

	for (int32 Idx = 0; Idx < LODDistance.Num(); ++Idx)
	{
		if (Distance <= LODDistance[Idx])
			return Idx;
	}
	return LODDistance.Num() - 1;
}

int32 AVoxelGround::GetChunkLODLevel(const FIntVector& ChunkCoord)
{
	int32 ChunkIdx = GetChunkIndex(ChunkCoord.X, ChunkCoord.Y, ChunkCoord.Z);
	return GetChunkLODLevel(ChunkIdx);
}

int32 AVoxelGround::GetChunkLODLevel(int32 ChunkIdx)
{
	if (ChunkDatas.IsValidIndex(ChunkIdx) == false)
		return 0;

	return ChunkDatas[ChunkIdx].LODLevel;
}

FNeighborLOD AVoxelGround::GetNeighborLOD(const FIntVector& ChunkCoord)
{
	FNeighborLOD NeighborLOD;
	NeighborLOD.XPlus = GetChunkLODLevel(ChunkCoord + FIntVector(1, 0, 0));
	NeighborLOD.YPlus = GetChunkLODLevel(ChunkCoord + FIntVector(0, 1, 0));
	NeighborLOD.ZPlus = GetChunkLODLevel(ChunkCoord + FIntVector(0, 0, 1));
	NeighborLOD.XMinus = GetChunkLODLevel(ChunkCoord + FIntVector(-1, 0, 0));
	NeighborLOD.YMinus = GetChunkLODLevel(ChunkCoord + FIntVector(0, -1, 0));
	NeighborLOD.ZMinus = GetChunkLODLevel(ChunkCoord + FIntVector(0, 0, -1));
	return NeighborLOD;
}

FNeighborLOD AVoxelGround::GetNeighborLOD(int32 ChunkIdx)
{
	return GetNeighborLOD(GetChunkCoord(ChunkIdx));
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
	NewChunk->UpdateMeshAsync(ChunkDatas[ChunkIdx].DensityValues, GetNeighborLOD(Coord), LODDistance.Num() - 1);
}

FVector AVoxelGround::GetPlayerLocation()
{
	ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	checkf(IsValid(PlayerCharacter) == true, TEXT("VoxelGround => Cannot find PlayerCharacter"));

	return PlayerCharacter->GetActorLocation();
}

void AVoxelGround::CheckPlayerChunk(const FVector& PlayerLocation)
{
	float ChunkSize = ChunkGridSize * ChunkVoxelSize;

	// 중심 청크 (플레이어가 현재 위치한 청크)
	FIntVector CurrentPlayerChunkCoord = FIntVector(
		FMath::FloorToInt(PlayerLocation.X / ChunkSize),
		FMath::FloorToInt(PlayerLocation.Y / ChunkSize),
		FMath::FloorToInt(PlayerLocation.Z / ChunkSize));

	if (CurrentPlayerChunkCoord != LastPlayerChunkCoord)
	{
		UpdateNearByChunks(CurrentPlayerChunkCoord);
		LastPlayerChunkCoord = CurrentPlayerChunkCoord;
	}
}

void AVoxelGround::UpdateNearByChunks(const FIntVector& PlayerChunkCoord)
{
	int32 MaxDistance = LODDistance.Last();
	FIntVector MinRange = PlayerChunkCoord - FIntVector(MaxDistance, MaxDistance, MaxDistance);
	FIntVector MaxRange = PlayerChunkCoord + FIntVector(MaxDistance, MaxDistance, MaxDistance);

	TSet<int32> UpdatedChunks;
	for (int32 Z = MinRange.Z; Z <= MaxRange.Z; ++Z)
	{
		for (int32 Y = MinRange.Y; Y <= MaxRange.Y; ++Y)
		{
			for (int32 X = MinRange.X; X <= MaxRange.X; ++X)
			{
				int32 ChunkIdx = GetChunkIndex(X, Y, Z);
				if (ChunkDatas.IsValidIndex(ChunkIdx) == false || IsValid(Chunks[ChunkIdx]) == false)
					continue;

				FIntVector ChunkCoord(X, Y, Z);
				ChunkDatas[ChunkIdx].LODLevel = GetLODLevelByPlayer(ChunkCoord, PlayerChunkCoord);
			}
		}
	}

	for (int32 Z = MinRange.Z; Z <= MaxRange.Z; ++Z)
	{
		for (int32 Y = MinRange.Y; Y <= MaxRange.Y; ++Y)
		{
			for (int32 X = MinRange.X; X <= MaxRange.X; ++X)
			{
				int32 ChunkIdx = GetChunkIndex(X, Y, Z);
				if (ChunkDatas.IsValidIndex(ChunkIdx) == false || IsValid(Chunks[ChunkIdx]) == false)
					continue;

				FIntVector ChunkCoord(X, Y, Z);
				Chunks[ChunkIdx]->UpdateMeshAsync(ChunkDatas[ChunkIdx].DensityValues, GetNeighborLOD(ChunkCoord), ChunkDatas[ChunkIdx].LODLevel);
			}
		}
	}
}

void AVoxelGround::UpdateChunkLODs()
{
	const FVector& PlayerLocation = GetPlayerLocation();
	CheckPlayerChunk(PlayerLocation);
}


