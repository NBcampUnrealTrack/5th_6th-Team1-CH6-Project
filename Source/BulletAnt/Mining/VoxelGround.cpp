#include "Mining/VoxelGround.h"
#include "Mining/VoxelGroundChunk.h"
#include "Kismet/GameplayStatics.h"
#include "Player/BACharacter.h"
#include "Mining/GroundSettingPreset.h"
#include "Components/BoxComponent.h"
#include "Framework/BAGameMode.h"
#include "Framework/BAGameState.h"
#include "GameFramework/PlayerState.h"
#include "EngineUtils.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Mining/VoxelGroundSubsystem.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Mining/BuriedComponent.h"

// -X, +X, -Y, +Y, -Z, +Z
const FIntVector AVoxelGround::NeighborOffsets[6] = { { -1, 0, 0 }, { 1, 0, 0 }, { 0, -1, 0 }, { 0, 1, 0 }, { 0, 0, -1 }, { 0, 0, 1 } };

AVoxelGround::AVoxelGround()
{
	PrimaryActorTick.bCanEverTick = true;

	USceneComponent* SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	BoundBox = CreateDefaultSubobject<UBoxComponent>(TEXT("BoundBox"));
	BoundBox->SetupAttachment(SceneRoot);
	BoundBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	BoundBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	BoundBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	BoundBox->SetGenerateOverlapEvents(true);
}

void AVoxelGround::BeginPlay()
{
	Super::BeginPlay();

	UVoxelGroundChunk::SetMaxLODLevel(LODDistance.Num());
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

void AVoxelGround::DigGround(TMap<EOreType, int32>& MinedOreMap, const FVector& WorldLocation, float Radius)
{
	if (GetNetMode() != NM_ListenServer)
		return;

	TRACE_CPUPROFILER_EVENT_SCOPE(DigGround);
	FVector RelativeLocation = WorldLocation - GetActorLocation();

	int32 MinX = FMath::FloorToInt((RelativeLocation.X - Radius) / ChunkSize);
	int32 MaxX = FMath::FloorToInt((RelativeLocation.X + Radius) / ChunkSize);
	int32 MinY = FMath::FloorToInt((RelativeLocation.Y - Radius) / ChunkSize);
	int32 MaxY = FMath::FloorToInt((RelativeLocation.Y + Radius) / ChunkSize);
	int32 MinZ = FMath::FloorToInt((RelativeLocation.Z - Radius) / ChunkSize);
	int32 MaxZ = FMath::FloorToInt((RelativeLocation.Z + Radius) / ChunkSize);

	UVoxelGroundSubsystem* GroundSubsystem = GetWorld()->GetSubsystem<UVoxelGroundSubsystem>();
	ensureMsgf(IsValid(GroundSubsystem) == true, TEXT("VoxelGroundSubststem is not valid"));

	for (int32 Z = MinZ; Z <= MaxZ; ++Z)
	{
		for (int32 Y = MinY; Y <= MaxY; ++Y)
		{
			for (int32 X = MinX; X <= MaxX; ++X)
			{
				int32 ChunkIdx = GetChunkIndex(X, Y, Z);
				if (ChunkDatas.IsValidIndex(ChunkIdx) == false || ChunkDatas[ChunkIdx].ChunkState == EChunkState::Air)
					continue;

				FVector ChunkOffset = FVector(X, Y, Z) * ChunkSize;
				FVoxelChunkEditData EditData;
				bool bDig = DigGround(ChunkIdx, ChunkOffset, WorldLocation, Radius, EditData, MinedOreMap);
				if (bDig == true)
				{
					GroundSubsystem->EnqueueEditData(EditData);
				}
			}
		}
	}

	for (const auto& Pair : MinedOreMap)
	{
		if (Pair.Key == EOreType::None)
			continue;

		ABAGameMode* GameMode = GetWorld()->GetAuthGameMode<ABAGameMode>();
		if (IsValid(GameMode) == true)
		{
			GameMode->MineOre(Pair.Key, Pair.Value);
		}
	}
}

bool AVoxelGround::DigGround(int32 ChunkIdx, const FVector& ChunkOffset, const FVector& WorldLocation, float Radius, FVoxelChunkEditData& OutData, TMap<EOreType, int32>& MinedOreMap)
{
	const FVector RelativeLocation = WorldLocation - GetActorLocation();
	const int32 ChunkPoints = Setting->ChunkGridSize + 1;
	const float RadiusSquared = Radius * Radius;

	bool bChunkModified = false;

	OutData.ChunkIdx = ChunkIdx;
	for (int32 Z = 0; Z < ChunkPoints; ++Z)
	{
		for (int32 Y = 0; Y < ChunkPoints; ++Y)
		{
			for (int32 X = 0; X < ChunkPoints; ++X)
			{
				FVector PointWorldPos = ChunkOffset + FVector(X, Y, Z) * Setting->ChunkVoxelSize;
				float DistSquared = FVector::DistSquared(RelativeLocation, PointWorldPos);

				if (DistSquared <= RadiusSquared + Setting->ChunkVoxelSize * Setting->ChunkVoxelSize)
				{
					int32 PointIdx = Z * ChunkPoints * ChunkPoints + Y * ChunkPoints + X;

					float DistRatio = FMath::Clamp(DistSquared / RadiusSquared, 0.0f, 1.0f);
					uint8 DistDensity = FMath::RoundToInt(DistRatio * 200.0f);

					uint8 CurrentDensity = GetChunkDensityValue(ChunkIdx, PointIdx);
					uint8 TargetDensity = FMath::Min(CurrentDensity, DistDensity);
					// 기반암(> 200)은 Dig 불가
					if (CurrentDensity <= 200 && TargetDensity < CurrentDensity)
					{
						FVoxelChangedResult OutResult;
						bool bChanged = ChangeChunkDensityValue(ChunkIdx, PointIdx, TargetDensity, OutResult);
						if (bChanged == true)
						{
							bChunkModified = true;
							OutData.PointEditDatas.Add({ PointIdx, TargetDensity });
							if (OutResult.bTypeChanged == true && OutResult.CurrType == EVoxelType::None)
							{
								EOreType OreType = EOreType::None;
								switch (OutResult.PrevType)
								{
									case EVoxelType::Vein:
										OreType = Setting->VeinOreType;
										break;
									case EVoxelType::Pillar:
										OreType = Setting->PillarOreType;
										break;
									default:
										break;
								}

								++MinedOreMap.FindOrAdd(OreType);
							}
						}
					}
				}
			}
		}
	}

	if (bChunkModified == true)
	{
		if (IsValid(Chunks[ChunkIdx]) == false)
		{
			SpawnChunk(ChunkIdx);
		}

		if (IsValid(Chunks[ChunkIdx]) == true)
		{
			UpdateChunkMeshImmediately(ChunkIdx, false);
		}
	}

	return bChunkModified;
}

bool AVoxelGround::MakeChunkSaveData(int32 ChunkIdx, FVoxelGroundChunkSaveData& OutData)
{
	if (ChunkDatas.IsValidIndex(ChunkIdx) == false)
		return false;

	const TArray<uint8>& RawData = ChunkDatas[ChunkIdx].DensityValues;
	int32 RawSize = RawData.Num();

	TArray<uint8> CompressedBuffer;
	CompressedBuffer.SetNum(FMath::Max(RawSize, 1024) * 2);
	int32 CompressedSize = CompressedBuffer.Num();

	bool bSuccess = FCompression::CompressMemory(
		NAME_Zlib,
		CompressedBuffer.GetData(),
		CompressedSize,
		RawData.GetData(),
		RawSize);

	if (bSuccess == true)
	{
		CompressedBuffer.SetNum(CompressedSize);

		OutData.ChunkIdx = ChunkIdx;
		OutData.ChunkState = ChunkDatas[ChunkIdx].ChunkState;
		OutData.CompressedDensityValues = MoveTemp(CompressedBuffer);
		OutData.UncompressedSize = RawSize;
	}

	return bSuccess;
}

bool AVoxelGround::LoadChunkSaveData(const FVoxelGroundChunkSaveData& Data)
{
	if (ChunkDatas.IsValidIndex(Data.ChunkIdx) == false)
		return false;

	TArray<uint8> DecompressedBuffer;
	DecompressedBuffer.SetNum(Data.UncompressedSize);

	bool bSuccess = FCompression::UncompressMemory(
		NAME_Zlib,
		DecompressedBuffer.GetData(),
		Data.UncompressedSize,
		Data.CompressedDensityValues.GetData(),
		Data.CompressedDensityValues.Num());

	if (bSuccess == true)
	{
		ChunkDatas[Data.ChunkIdx].ChunkState = Data.ChunkState;
		ChunkDatas[Data.ChunkIdx].DensityValues = MoveTemp(DecompressedBuffer);
	}

	return bSuccess;
}

void AVoxelGround::EnqueueChunkUpdateResult(FChunkUpdateResult&& Result)
{
	ChunkUpdateResultQueue.Enqueue(Forward<FChunkUpdateResult>(Result));
}

void AVoxelGround::InitializeGround(int32 InSeed, const UGroundSettingPreset* InSetting)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(InitializeGround);
	
	Seed = InSeed;
	Setting = InSetting;

	ensureMsgf(IsValid(Setting) == true, TEXT("Setting is not valid"));

	ChunkSize = Setting->ChunkGridSize * Setting->ChunkVoxelSize;
	CellSize = ChunkSize * 0.25f;
	BuriedRegionSize = ChunkSize * 2.0f;

	const FVector& GroundSize = Setting->GroundSize;
	ChunkRangeMax.X = FMath::CeilToInt(GroundSize.X * 0.5f / ChunkSize) + 1;
	ChunkRangeMax.Y = FMath::CeilToInt(GroundSize.Y * 0.5f / ChunkSize) + 1;
	ChunkRangeMax.Z = -1;
	ChunkRangeMin.X = -(ChunkRangeMax.X + 1);
	ChunkRangeMin.Y = -(ChunkRangeMax.Y + 1);
	ChunkRangeMin.Z = -(FMath::CeilToInt(GroundSize.Z / ChunkSize) + 1);
	GridWidth = ChunkRangeMax - ChunkRangeMin + FIntVector(1, 1, 1);

	PlaceObjectsByCell();

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

	if (IsValid(BoundBox) == true)
	{
		SetBoundBox();

		BoundBox->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnPlayerEnter);
		BoundBox->OnComponentEndOverlap.AddDynamic(this, &ThisClass::OnPlayerExit);
	}
}

void AVoxelGround::InitializeChunkData(int32 ChunkIdx)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(InitializeChunkData);

	ChunkDatas[ChunkIdx].LODLevel = LODDistance.Num();
	ChunkDatas[ChunkIdx].ChunkState = EChunkState::Ground;

	int32 ChunkPoints = Setting->ChunkGridSize + 1;
	int32 TotalPoints = ChunkPoints * ChunkPoints * ChunkPoints;

	InitializeChunkDensities(ChunkIdx);

	if (ChunkDatas[ChunkIdx].ChunkState != EChunkState::Complex)
	{
		ChunkDatas[ChunkIdx].DensityValues.Empty(0);
		ChunkDatas[ChunkIdx].VoxelTypes.Empty(0);
	}
}

void AVoxelGround::InitializeChunkDensities(int32 ChunkIdx)
{
	FIntVector ChunkCoord = GetChunkCoord(ChunkIdx);
	FVector ChunkOffset = GetChunkOffset(ChunkCoord.X, ChunkCoord.Y, ChunkCoord.Z);

	int32 ChunkPoints = Setting->ChunkGridSize + 1;
	int32 TotalPoints = ChunkPoints * ChunkPoints * ChunkPoints;

	TArray<uint8> TempDensities;
	TArray<EVoxelType> TempVoxelTypes;
	TempDensities.SetNumUninitialized(TotalPoints);
	TempVoxelTypes.SetNumUninitialized(TotalPoints);

	// 쓰레드 안전을 위해 Atomic 사용
	std::atomic<int32> AtomicGroundVoxelCount(0);
	std::atomic<int32> AtomicBedRockCount(0);

	const FVector& GroundSize = Setting->GroundSize;
	const FVector BoundMinRange = FVector(-GroundSize.X * 0.48f, -GroundSize.Y * 0.48f, -GroundSize.Z);
	const FVector BoundMaxRange = FVector(GroundSize.X * 0.48f, GroundSize.Y * 0.48f, -100.0f);
	const FVector Center = (BoundMinRange + BoundMaxRange) * 0.5f;
	const FVector HalfSize = (BoundMaxRange - BoundMinRange) * 0.5f;

	const float ChunkVoxelSize = Setting->ChunkVoxelSize;
	const uint8 IsoLevel = Setting->IsoLevel;
	const int32 LocalSeed = Seed;

	ParallelFor(ChunkPoints, [&](int32 Z)
		{
			int32 LocalGroundVoxelCount = 0;
			int32 LocalBedRockCount = 0;

			for (int32 Y = 0; Y < ChunkPoints; ++Y)
			{
				for (int32 X = 0; X < ChunkPoints; ++X)
				{
					FVector WorldPos = ChunkOffset + FVector(X, Y, Z) * ChunkVoxelSize;

					uint8 FinalDensity = 0;
					EVoxelType VoxelType = EVoxelType::None;

					int32 PointIdx = Z * ChunkPoints * ChunkPoints + Y * ChunkPoints + X;

					// 전체 지형 Bound - Bound 외부는 반드시 공기
					bool bInBound =
						WorldPos.X >= BoundMinRange.X && WorldPos.X <= BoundMaxRange.X &&
						WorldPos.Y >= BoundMinRange.Y && WorldPos.Y <= BoundMaxRange.Y &&
						WorldPos.Z >= BoundMinRange.Z && WorldPos.Z <= BoundMaxRange.Z;

					auto ApplyBuried =
						[&](const FVector& WorldPos)
						{
							FIntVector BuriedRegion = FIntVector(
								FMath::FloorToInt(WorldPos.X / BuriedRegionSize),
								FMath::FloorToInt(WorldPos.Y / BuriedRegionSize),
								FMath::FloorToInt(WorldPos.Z / BuriedRegionSize));
							if (const FBuriedLocationData* Data = BuriedLocationDatas.Find(BuriedRegion))
							{
								if (Data->bCarveDensity == false)
									return;

								const auto& BoundInfos = Data->BoundInfos;
								for (const auto& BoundInfo : BoundInfos)
								{
									FVector LocalPos = BoundInfo.Transform.InverseTransformPosition(WorldPos);
									if (FMath::Abs(LocalPos.X) <= BoundInfo.Extent.X &&
										FMath::Abs(LocalPos.Y) <= BoundInfo.Extent.Y &&
										FMath::Abs(LocalPos.Z) <= BoundInfo.Extent.Z)
									{
										VoxelType = EVoxelType::None;
										FinalDensity = 0;
									}
								}
							}
						};

					auto ApplyVein =
						[&](const FVector& WorldPos)
						{
							FIntVector Cell = FIntVector(
								FMath::FloorToInt(WorldPos.X / CellSize),
								FMath::FloorToInt(WorldPos.Y / CellSize),
								FMath::FloorToInt(WorldPos.Z / CellSize));

							if (const FVeinLocationData* Data = VeinLocationDatas.Find(Cell))
							{
								float Dist = FVector::Distance(WorldPos, Data->VeinLocation);

								if (Dist > Data->VeinRadius)
									return;

								float t = 1.0f - (Dist / Data->VeinRadius);

								float RoughNoise = FMath::PerlinNoise3D(WorldPos * 0.02f);
								float Roughness = RoughNoise * 4.0f;

								VoxelType = EVoxelType::Vein;
								FinalDensity = 200;// (uint8)FMath::Clamp(FinalDensity + 80, 0, 200);
							}
						};

					auto ApplyPillar =
						[&](const FVector& WorldPos)
						{
							FIntVector Cell = FIntVector(
								FMath::FloorToInt(WorldPos.X / CellSize),
								FMath::FloorToInt(WorldPos.Y / CellSize),
								FMath::FloorToInt(WorldPos.Z / CellSize));

							if (const FPillarLocationData* Data = PillarLocationDatas.Find(Cell))
							{
								FVector Delta = WorldPos - Data->PillarLocation;
								float AlongNormal = FVector::DotProduct(Delta, Data->PillarNormal);
								float Radial = (Delta - (Data->PillarNormal * AlongNormal)).Length();

								if (AlongNormal < 0 || Radial > Data->PillarRadius)
									return;

								float Distance = Radial / Data->PillarRadius;
								float DistanceValue = (1.0f - Distance) * (1.0f - (AlongNormal / Data->PillarHeight) * 0.2f);

								VoxelType = EVoxelType::Pillar;
								FinalDensity = (uint8)FMath::Clamp(DistanceValue * 200, 0, 200);
							}
						};

					// Bound 내부라면
					if (bInBound == true)
					{
						FVector LocalPos = WorldPos - Center;
						FVector AbsLocalPos = LocalPos.GetAbs();
						float DistXYSquared = LocalPos.X * LocalPos.X + LocalPos.Y * LocalPos.Y;

						float MaxAxisRatio = FMath::Max3(
							AbsLocalPos.X / HalfSize.X,
							AbsLocalPos.Y / HalfSize.Y,
							AbsLocalPos.Z / HalfSize.Z);

						bool bBedRock = false;
						const static float BedRockNoiseWeight = 0.02f;
						const static float BedRockThickness = 0.04f;
						// MaxAxisRatio가 기반암이 생성 가능한 수치 이내라면, 위쪽 중앙 입구 지역이 아니라면
						bool bIsBedRockRange = MaxAxisRatio >= (1 - BedRockNoiseWeight - BedRockThickness);
						if (bIsBedRockRange == true)
						{
							float BedRockNoiseValue = FMath::PerlinNoise3D(WorldPos * 0.0008f);
							float DistortedRatio = MaxAxisRatio + BedRockNoiseValue * BedRockNoiseWeight;
							float BedRockDensity = FMath::Clamp((DistortedRatio - (1.0f - BedRockThickness)) / BedRockThickness, 0.0f, 1.0f);

							// 기반암이면 Density 200 넘도록 Clamp, 기반암 아닌 경우엔 광물 계산
							if (BedRockDensity > 0.0f)
							{
								bBedRock = true;
								VoxelType = EVoxelType::BedRock;
								FinalDensity = (uint8)FMath::Clamp(FMath::RoundToInt(201 + BedRockDensity * 54.0f), 201, 255);
							}
						}

						if (bBedRock == false)
						{
							FinalDensity = GetBaseDensity(WorldPos);
							ApplyBuried(WorldPos);

							bool bGround = FinalDensity > IsoLevel;
							if (bGround == true)
							{
								VoxelType = EVoxelType::NormalRock;

								// 땅이라면, Vein 계산
								ApplyVein(WorldPos);
							}
							else
							{
								// 공기라면, Pillar 계산
								ApplyPillar(WorldPos);
							}
						}

						const float EntranceRadius = 400.0f;
						//bool bEntrance = bIsBedRockRange == true && (WorldPos.Z > Center.Z && DistXYSquared <= (EntranceRadius * EntranceRadius));
						bool bEntrance = bIsBedRockRange == true && AbsLocalPos.X <= EntranceRadius && AbsLocalPos.Y <= EntranceRadius;
						if (bEntrance == true)
						{
							// Bedrock 구분 방법 denstiy로 되어 있는 걸 전부 EVoxelType으로 교체 후에 원형 구멍으로 변경
							FinalDensity = 0;
							VoxelType = EVoxelType::None;
							//FinalDensity = (uint8)FMath::Clamp(FMath::RoundToInt((DistXYSquared / (EntranceRadius * EntranceRadius)) * 200.0f), 0, 200);
							//VoxelType = FinalDensity > IsoLevel ? VoxelType : EVoxelType::None;
						}
					}

					TempDensities[PointIdx] = FinalDensity;
					TempVoxelTypes[PointIdx] = VoxelType;

					if (FinalDensity > IsoLevel)
					{
						++LocalGroundVoxelCount;

						if (FinalDensity > 200)
						{
							++LocalBedRockCount;
						}
					}
				}
			}

			AtomicGroundVoxelCount += LocalGroundVoxelCount;
			AtomicBedRockCount += LocalBedRockCount;
		});

	int32 GroundVoxelCount = AtomicGroundVoxelCount.load();
	int32 BedRockCount = AtomicBedRockCount.load();

	// 모두 땅이거나 모두 공기이면(청크가 단일 상태라면) 데이터도 저장 X
	// 기반암이 섞인 땅이어도 Complex
	EChunkState NewChunkState = EChunkState::Complex;
	if (GroundVoxelCount <= 0 ||
		(GroundVoxelCount == TotalPoints && BedRockCount == TotalPoints))
	{
		NewChunkState = GroundVoxelCount == 0 ? EChunkState::Air : EChunkState::Ground;
	}
	ChunkDatas[ChunkIdx].ChunkState = NewChunkState;

	ChunkDatas[ChunkIdx].GroundVoxelCount = GroundVoxelCount;
	ChunkDatas[ChunkIdx].DensityValues = MoveTemp(TempDensities);
	ChunkDatas[ChunkIdx].VoxelTypes = MoveTemp(TempVoxelTypes);
}

void AVoxelGround::PlaceObjectsByCell()
{
	const int32 ChunkPoints = Setting->ChunkGridSize + 1;
	const float VoxelSize = Setting->ChunkVoxelSize;								// Buried Actor는 Cell의 4배까지 크다고 가정 
	const auto& BuriedActorClasses = Setting->BuriedActorClasses;
	const uint8 IsoLevel = Setting->IsoLevel;
	const FVector& GroundSize = Setting->GroundSize;

	// Region
	FIntVector BuriedRegionRangeMin = FIntVector(
		FMath::Floor(ChunkRangeMin.X * 0.5f) + 2,
		FMath::Floor(ChunkRangeMin.Y * 0.5f) + 2,
		FMath::Floor(ChunkRangeMin.Z * 0.5f) + 2);
	FIntVector BuriedRegionRangeMax = FIntVector(
		FMath::Floor(ChunkRangeMax.X * 0.5f) - 2,
		FMath::Floor(ChunkRangeMax.Y * 0.5f) - 2,
		FMath::Floor(ChunkRangeMax.Z * 0.5f) - 1);

	TArray<FIntVector> BuriedRegions;
	for (int32 Z = BuriedRegionRangeMin.Z; Z <= BuriedRegionRangeMax.Z; ++Z)
	{
		for (int32 Y = BuriedRegionRangeMin.Y; Y <= BuriedRegionRangeMax.Y; ++Y)
		{
			for (int32 X = BuriedRegionRangeMin.X; X <= BuriedRegionRangeMax.X; ++X)
			{
				BuriedRegions.Add(FIntVector(X, Y, Z));
			}
		}
	}

	FRandomStream StreamBury(Seed);
	int32 TotalBuryRegions = BuriedRegions.Num();
	for (int32 Idx = 0; Idx < TotalBuryRegions; ++Idx)
	{
		int32 SwapIdx = StreamBury.RandRange(Idx, TotalBuryRegions - 1);
		BuriedRegions.Swap(Idx, SwapIdx);
	}

	int32 RegionIdx = 0;
	for (const auto& [BuriedActorClass, BuriedActorCount] : BuriedActorClasses)
	{
		if (IsValid(BuriedActorClass) == false)
			continue;

		FActorSpawnParameters TempSpawnParams;
		TempSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		AActor* TempBuriedActor = GetWorld()->SpawnActor<AActor>(BuriedActorClass, TempSpawnParams);
		if (IsValid(TempBuriedActor) == false)
			continue;

		UBuriedComponent* BuriedComponent = TempBuriedActor->GetComponentByClass<UBuriedComponent>();
		if (IsValid(BuriedComponent) == false)
			continue;

		bool bCarveDensity = BuriedComponent->IsCarveDensity();

		for (int32 Count = 0; Count < BuriedActorCount && RegionIdx < BuriedRegions.Num(); ++RegionIdx)
		{
			TArray<FBuryBoundInfo> BuryBoundInfos;
			FRotator Rotation = FRotator(0.0f, StreamBury.FRandRange(0.0f, 360.0f), 0.0f);
			FTransform Transform(Rotation);
			BuriedComponent->GetPredictedBoundInfos(BuryBoundInfos, Transform);
			FBox TotalBound(EForceInit::ForceInit);
			for (const auto& BoundInfo : BuryBoundInfos)
			{
				TotalBound += BoundInfo.Bound;
			}

			const FIntVector& BuriedRegion = BuriedRegions[RegionIdx];
			FVector HalfBoundSize = (TotalBound.Max - TotalBound.Min) * 0.5f;
			FVector Available = FVector::OneVector * BuriedRegionSize - HalfBoundSize * 2.0f;
			FVector CandidateRangeMin = FVector(BuriedRegion) * BuriedRegionSize + HalfBoundSize;
			FVector CandidateRangeMax = CandidateRangeMin + Available;
			for (int32 CandidateCount = 0; CandidateCount < 16; ++CandidateCount)
			{
				FVector Candidate = FVector(
					StreamBury.FRandRange(CandidateRangeMin.X, CandidateRangeMax.X),
					StreamBury.FRandRange(CandidateRangeMin.Y, CandidateRangeMax.Y),
					StreamBury.FRandRange(CandidateRangeMin.Z, CandidateRangeMax.Z));

				uint8 BaseDensity = GetBaseDensity(Candidate);
				if (BaseDensity <= IsoLevel)
					continue;

				FBuriedLocationData LocationData;
				FTransform BuryTransform(Rotation, Candidate);
				LocationData.BuriedTransform = BuryTransform;
				LocationData.BuriedActorClass = BuriedActorClass;
				LocationData.bCarveDensity = bCarveDensity;
				BuriedComponent->GetPredictedBoundInfos(LocationData.BoundInfos, BuryTransform);

				/*DrawDebugSphere(
					GetWorld(),
					Candidate,
					400.0f,
					16,
					FColor::Red,
					true);*/

				BuriedLocationDatas.Add(BuriedRegion, LocationData);
				++Count;
				break;
			}
		}

		TempBuriedActor->Destroy();
	}

	// Cell: Vein, Pillar
	FIntVector CellRangeMin = FIntVector(
		(ChunkRangeMin.X + 3) * 4 + 2,
		(ChunkRangeMin.Y + 3) * 4 + 2,
		(ChunkRangeMin.Z + 1) * 4 + 2);
	FIntVector CellRangeMax = FIntVector(
		(ChunkRangeMax.X - 3) * 4,
		(ChunkRangeMax.Y - 3) * 4,
		(ChunkRangeMax.Z) * 2 + 2);

	TArray<FIntVector> Cells;
	for (int32 Z = CellRangeMin.Z; Z <= CellRangeMax.Z; ++Z)
	{
		for (int32 Y = CellRangeMin.Y; Y <= CellRangeMax.Y; ++Y)
		{
			for (int32 X = CellRangeMin.X; X <= CellRangeMax.X; ++X)
			{
				Cells.Add(FIntVector(X, Y, Z));
			}
		}
	}

	FRandomStream StreamCell(Seed + 1000);
	int32 TotalCells = Cells.Num();
	for (int32 Idx = 0; Idx < TotalCells; ++Idx)
	{
		int32 SwapIdx = StreamCell.RandRange(Idx, TotalCells - 1);
		Cells.Swap(Idx, SwapIdx);
	}

	const int32 MaxPillarCount = Setting->PillarCount;
	for (int32 PillarCount = 0, CellIdx = 0; PillarCount < MaxPillarCount && CellIdx < Cells.Num(); ++CellIdx)
	{
		const FIntVector& Cell = Cells[CellIdx];
		FIntVector ChunkIncludeCell = FIntVector(
			FMath::Floor(Cell.X * 0.5f),
			FMath::Floor(Cell.Y * 0.5f),
			FMath::Floor(Cell.Z * 0.5f));
		if (BuriedLocationDatas.Contains(ChunkIncludeCell) == true)
			continue;

		int32 Hash = HashCombine(HashCombine(Cell.X, Cell.Y), HashCombine(Cell.Z, Seed));
		FRandomStream Stream(Hash);

		FVector CellCenter = FVector(
			(Cell.X + 0.5f) * CellSize,
			(Cell.Y + 0.5f) * CellSize,
			(Cell.Z + 0.5f) * CellSize);
		float PillarRadius = 240.0f;
		float PillarHeight = 320.0f + 180.0f * Stream.FRand();
		float Available = CellSize - PillarRadius * 2.0f;
		FVector CandidateRangeMin = FVector(CellSize) * BuriedRegionSize + PillarRadius;
		FVector CandidateRangeMax = CandidateRangeMin + Available;

		for (int32 Count = 0; Count < 16; ++Count)
		{
			FVector Candidate = FVector(
				Cell.X * CellSize + PillarRadius + (Available * Stream.FRand()),
				Cell.Y * CellSize + PillarRadius + (Available * Stream.FRand()),
				Cell.Z * CellSize + PillarRadius + (Available * Stream.FRand()));

			bool bIsNearSurface = FMath::Abs(GetBaseDensity(Candidate) - IsoLevel) < 8;
			if (bIsNearSurface == false)
				continue;

			float Dx = GetBaseDensity(Candidate + FVector(VoxelSize, 0, 0)) - GetBaseDensity(Candidate - FVector(VoxelSize, 0, 0));
			float Dy = GetBaseDensity(Candidate + FVector(0, VoxelSize, 0)) - GetBaseDensity(Candidate - FVector(0, VoxelSize, 0));
			float Dz = GetBaseDensity(Candidate + FVector(0, 0, VoxelSize)) - GetBaseDensity(Candidate - FVector(0, 0, VoxelSize));

			FVector Normal = -FVector(Dx, Dy, Dz).GetSafeNormal();
			FVector DirCenter = CellCenter - Candidate;
			if ((DirCenter.X * Normal.X < 0.0f) ||
				(DirCenter.Y * Normal.Y < 0.0f) ||
				(DirCenter.Z * Normal.Z < 0.0f))
				continue;

			FPillarLocationData LocationData;
			LocationData.PillarLocation = Candidate;
			LocationData.PillarRadius = PillarRadius;
			LocationData.PillarHeight = PillarHeight;
			LocationData.PillarNormal = Normal;
			PillarLocationDatas.Add(Cell, LocationData);
			++PillarCount;

			/*DrawDebugSphere(
				GetWorld(),
				Candidate,
				400.0f,
				4,
				FColor::Red,
				true);*/
			break;
		}
	}

	const int32 MaxVeinCount = Setting->VeinCount;
	for (int32 VeinCount = 0, CellIdx = 0; VeinCount < MaxVeinCount && CellIdx < Cells.Num(); ++CellIdx)
	{
		const FIntVector& Cell = Cells[CellIdx];
		FIntVector ChunkIncludeCell = FIntVector(
			FMath::Floor(Cell.X * 0.5f),
			FMath::Floor(Cell.Y * 0.5f),
			FMath::Floor(Cell.Z * 0.5f));
		if (BuriedLocationDatas.Contains(ChunkIncludeCell) == true || PillarLocationDatas.Contains(Cell) == true)
			continue;

		int32 Hash = HashCombine(HashCombine(Cell.X, Cell.Y), HashCombine(Cell.Z, Seed));
		FRandomStream Stream(Hash);

		// 반지름 = 100 ~ 360 / 지름 = 200 ~ 720
		float Radius = 100.0f +Stream.FRand() * 260.0f;
		float Available = CellSize - Radius * 2.0f;
		for (int32 Count = 0; Count < 16; ++Count)
		{
			FVector Candidate = FVector(
				Cell.X * CellSize + Radius + (Available * Stream.FRand()),
				Cell.Y * CellSize + Radius + (Available * Stream.FRand()),
				Cell.Z * CellSize + Radius + (Available * Stream.FRand()));

			uint8 BaseDensity = GetBaseDensity(Candidate);
			if (BaseDensity <= IsoLevel)
				continue;

			FVeinLocationData LocationData;
			LocationData.VeinLocation = Candidate;
			LocationData.VeinRadius = Radius;
			VeinLocationDatas.Add(Cell, LocationData);
			++VeinCount;

			/*DrawDebugSphere(
				GetWorld(),
				Candidate,
				400.0f,
				4,
				FColor::Yellow,
				true);*/
			break;
		}
	}
}

uint8 AVoxelGround::GetBaseDensity(const FVector& WorldPos)
{
	const static float CaveFrequency = 0.0006f;
	const static float CaveThreshold = 0.2f;

	float Density = 200.0f;

	FRandomStream Stream(Seed);
	FVector SeedOffset = Stream.GetUnitVector() * 20000.0f;
	FVector SamplePos = WorldPos + SeedOffset;
	FVector CavePos = SamplePos * CaveFrequency;
	CavePos.Z *= 1.4f;
	float CaveNoise0 = FMath::Abs(FMath::PerlinNoise3D(CavePos));
	float CaveNoise1 = FMath::Abs(FMath::PerlinNoise3D(CavePos + FVector::OneVector * 100.0f));
	float CaveNoise2 = FMath::Abs(FMath::PerlinNoise3D(CavePos + FVector::OneVector * 200.0f));
	float CaveValue = FMath::Sqrt(CaveNoise0 * CaveNoise0 + CaveNoise1 * CaveNoise1 + CaveNoise2 * CaveNoise2);

	if (CaveValue > CaveThreshold)
	{
		float Alpha = FMath::Clamp((CaveValue - CaveThreshold) / (1.0f - CaveThreshold), 0.0f, 1.0f);
		Density -= Alpha * 200.0f;
	}

	return (uint8)FMath::Clamp(Density, 0, 200);
}

void AVoxelGround::SpawnChunk(int32 ChunkIdx)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(SpawnChunk);

	if (ChunkDatas[ChunkIdx].ChunkState != EChunkState::Complex || ChunkDatas[ChunkIdx].DensityValues.IsEmpty() == true)
		return;

	FIntVector Coord = GetChunkCoord(ChunkIdx);
	FVector RelativeLocation = GetChunkOffset(Coord.X, Coord.Y, Coord.Z);

	UVoxelGroundChunk* NewChunk = NewObject<UVoxelGroundChunk>(this);
	NewChunk->SetIsReplicated(false);
	NewChunk->RegisterComponent();
	NewChunk->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
	NewChunk->SetMaterial(0, Setting->GroundMaterial);
	NewChunk->bUseAsyncCooking = true;
	NewChunk->bCastVolumetricTranslucentShadow = false;
	NewChunk->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	NewChunk->SetCollisionResponseToAllChannels(ECR_Ignore);
	NewChunk->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	NewChunk->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Block);
	NewChunk->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	NewChunk->SetCollisionResponseToChannel(ECC_Camera, ECR_Block);
	NewChunk->SetCollisionResponseToChannel(ECC_GameTraceChannel5, ECR_Block);
	NewChunk->SetComplexAsSimpleCollisionEnabled(true);
	NewChunk->SetGenerateOverlapEvents(false);
	NewChunk->SetRelativeLocation(RelativeLocation);
	AddInstanceComponent(NewChunk);
	Chunks[ChunkIdx] = NewChunk;
	
	NewChunk->InitializeChunk(Setting->ChunkGridSize, Setting->ChunkVoxelSize, Setting->IsoLevel);
	NewChunk->CoordV = Coord;
	NewChunk->ChunkIdxV = ChunkIdx;

	//NewChunk->SetVisibility(false);
}

void AVoxelGround::UpdateNearByChunks(const TSet<FIntVector>& CoordsChanged)
{
	if (CoordsChanged.IsEmpty() == true)
		return;

	TSet<int32> ChunkIdxsToCheck;

	int32 MaxDistWithMargin = LODDistance.Last() + LODDistMargin;

	for (const FIntVector& CoordChanged : CoordsChanged)
	{
		int32 MaxDistance = MaxDistWithMargin;
		FIntVector MinRange = CoordChanged - FIntVector(MaxDistance, MaxDistance, MaxDistance);
		FIntVector MaxRange = CoordChanged + FIntVector(MaxDistance, MaxDistance, MaxDistance);

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
	}

	for (int32 ChunkIdx : ChunkIdxsToCheck)
	{
		FIntVector ChunkCoord = GetChunkCoord(ChunkIdx);

		int32 OldLODLevel = ChunkDatas[ChunkIdx].LODLevel;
		int32 NewLODLevel = LODDistance.Num();

		// LastPlayerCoords 갱신 후이므로 현재 PlayerCoords
		for (const FIntVector& PlayerChunkCoord : LastPlayerCoords)
		{
			int32 CurrentPlayerLODLevel = GetLODLevelByPlayer(ChunkCoord, PlayerChunkCoord, OldLODLevel);
			NewLODLevel = FMath::Min(NewLODLevel, CurrentPlayerLODLevel);
		}

		if (NewLODLevel != OldLODLevel)
		{
			ChunkDatas[ChunkIdx].LODLevel = NewLODLevel;
			UpdateChunkMesh(ChunkIdx, true);
		}
	}
}

void AVoxelGround::UpdateChunkLODs()
{
	TSet<FIntVector> CoordsChanged = LastPlayerCoords;
	TSet<FIntVector> CurrentPlayerCoords;

	ABAGameState* GS = GetWorld()->GetGameState<ABAGameState>();
	ensureMsgf(IsValid(GS) == true, TEXT("GameState is not valid"));

	const auto& PlayerStates = GS->PlayerArray;
	for (APlayerState* PS : PlayerStates)
	{
		if (IsValid(PS) == false)
			continue;

		APawn* CharacterPawn = PS->GetPawn();
		if (IsValid(CharacterPawn) == false)
			continue;

		FVector RelativePlayerLocation = CharacterPawn->GetActorLocation() - GetActorLocation();

		// 중심 청크 (플레이어가 현재 위치한 청크)
		FIntVector CurrentPlayerChunkCoord = FIntVector(
			FMath::FloorToInt(RelativePlayerLocation.X / ChunkSize),
			FMath::FloorToInt(RelativePlayerLocation.Y / ChunkSize),
			FMath::FloorToInt(RelativePlayerLocation.Z / ChunkSize));

		CurrentPlayerCoords.Add(CurrentPlayerChunkCoord);
	}

	for (const auto& CurrentChunkCoord : CurrentPlayerCoords)
	{
		// 기존에도 플레이어가 있던 청크는 검사 X
		// 위치가 변경된 플레이어의 이동 전, 이동 후 청크만 검사
		if (CoordsChanged.Contains(CurrentChunkCoord) == true)
		{
			CoordsChanged.Remove(CurrentChunkCoord);
		}
		else
		{
			CoordsChanged.Add(CurrentChunkCoord);
		}
	}
	LastPlayerCoords = CurrentPlayerCoords;

	if (CoordsChanged.IsEmpty() == false)
	{
		UpdateNearByChunks(CoordsChanged);
	}
}

bool AVoxelGround::ChangeChunkDensityValue(int32 ChunkIdx, int32 PointIdx, int32 NewDensityValue, FVoxelChangedResult& OutResult)
{
	if (ChunkDatas.IsValidIndex(ChunkIdx) == false)
		return false;

	int32 ChunkPoints = Setting->ChunkGridSize + 1;
	int32 TotalPoints = ChunkPoints * ChunkPoints * ChunkPoints;
	if (PointIdx < 0 || PointIdx >= TotalPoints)
		return false;

	bool bIsGroundNew = NewDensityValue > Setting->IsoLevel;
	if (ChunkDatas[ChunkIdx].DensityValues.IsEmpty() == true)
	{
		ensureMsgf(ChunkDatas[ChunkIdx].ChunkState != EChunkState::Complex, TEXT("Complex Chunk, but no density values"));
		if ((bIsGroundNew == true && ChunkDatas[ChunkIdx].ChunkState == EChunkState::Ground) ||
			(bIsGroundNew == false && ChunkDatas[ChunkIdx].ChunkState == EChunkState::Air))
			return false;

		InitializeChunkDensities(ChunkIdx);
	}

	// 기반암이면 변경 X
	if (ChunkDatas[ChunkIdx].DensityValues[PointIdx] > 200)
		return false;

	const uint8 IsoLevel = Setting->IsoLevel;

	OutResult.PrevDensity = ChunkDatas[ChunkIdx].DensityValues[PointIdx];
	OutResult.CurrDensity = NewDensityValue;
	OutResult.PrevType = ChunkDatas[ChunkIdx].VoxelTypes[PointIdx];
	OutResult.CurrType = NewDensityValue <= IsoLevel ? EVoxelType::None : OutResult.PrevType;
	OutResult.bTypeChanged = OutResult.PrevType != OutResult.CurrType;

	bool bIsGroundOld = ChunkDatas[ChunkIdx].DensityValues[PointIdx] > IsoLevel;
	ChunkDatas[ChunkIdx].DensityValues[PointIdx] = NewDensityValue;
	if (bIsGroundOld != bIsGroundNew)
	{
		ChunkDatas[ChunkIdx].GroundVoxelCount += bIsGroundNew == true ? 1 : -1;
		if (ChunkDatas[ChunkIdx].GroundVoxelCount != 0 && ChunkDatas[ChunkIdx].GroundVoxelCount != TotalPoints)
		{
			ChunkDatas[ChunkIdx].ChunkState = EChunkState::Complex;
		}
		else
		{
			ChunkDatas[ChunkIdx].ChunkState = ChunkDatas[ChunkIdx].GroundVoxelCount == 0 ? EChunkState::Air : EChunkState::Ground;
			ChunkDatas[ChunkIdx].DensityValues.Empty(0);
			ChunkDatas[ChunkIdx].VoxelTypes.Empty(0);
		}
	}

	return true;
}

uint8 AVoxelGround::GetChunkDensityValue(int32 ChunkIdx, int32 PointIdx)
{
	if (ChunkDatas.IsValidIndex(ChunkIdx) == false)
		return 0;

	if (ChunkDatas[ChunkIdx].DensityValues.IsValidIndex(PointIdx) == true)
		return ChunkDatas[ChunkIdx].DensityValues[PointIdx];

	if (ChunkDatas[ChunkIdx].ChunkState == EChunkState::Ground)
		return 200;

	return 0;
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
	return FVector(X, Y, Z) * ChunkSize;
}

int32 AVoxelGround::GetLODLevelByPlayer(const FIntVector& ChunkCoord, const FIntVector& PlayerChunkCoord, int32 CurrentLODLevel)
{
	int32 DistX = FMath::Abs(ChunkCoord.X - PlayerChunkCoord.X);
	int32 DistY = FMath::Abs(ChunkCoord.Y - PlayerChunkCoord.Y);
	int32 DistZ = FMath::Abs(ChunkCoord.Z - PlayerChunkCoord.Z);
	int32 Distance = FMath::Max3(DistX, DistY, DistZ);

	for (int32 Idx = 0; Idx < LODDistance.Num(); ++Idx)
	{
		int32 Threshold = LODDistance[Idx];
		if (CurrentLODLevel <= Idx)
		{
			Threshold += LODDistMargin;
		}

		if (Distance <= Threshold)
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
				Chunks[ChunkIdx]->CalculateMeshDataAsync(NextChunkUpdateID++, this, ChunkDatas[ChunkIdx].DensityValues, ChunkDatas[ChunkIdx].VoxelTypes, GetNeighborLOD(ChunkIdx), GetChunkLODLevel(ChunkIdx));
			}
			ChunkMeshDirties[ChunkIdx] = false;
			++Count;
		}
	}

	//UKismetSystemLibrary::PrintString(GetWorld(), *FString::FromInt(Count));
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
				Chunks[ChunkIdx]->CalculateMeshDataAsync(NextChunkUpdateID++, this, ChunkDatas[ChunkIdx].DensityValues, ChunkDatas[ChunkIdx].VoxelTypes, GetNeighborLOD(ChunkIdx), GetChunkLODLevel(ChunkIdx));
			}
			PriorityChunkMeshDirties[ChunkIdx] = false;
		}
	}
}

void AVoxelGround::ApplyEditPacket(const FVoxelChunkEditPacket& Packet)
{
	for (const FVoxelChunkEditData& Data : Packet.ChunkEditDatas)
	{
		EditGroundChunk(Data);
	}
}

void AVoxelGround::EditGroundChunk(const FVoxelChunkEditData& Data)
{
	if (GetNetMode() != NM_Client)
		return;

	if (ChunkDatas.IsValidIndex(Data.ChunkIdx) == false)
		return;

	int32 ChunkIdx = Data.ChunkIdx;
	int32 Count = Data.PointEditDatas.Num();
	if (Count > 0)
	{
		for (const FVoxelPointEditData& PointData : Data.PointEditDatas)
		{
			FVoxelChangedResult OutResult;
			ChangeChunkDensityValue(ChunkIdx, PointData.VoxelIndex, PointData.NewDensityValue, OutResult);
		}

		if (ChunkDatas[ChunkIdx].ChunkState == EChunkState::Complex)
		{
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
}

void AVoxelGround::SetBoundBox()
{
	const FVector& GroundSize = Setting->GroundSize;
	const FVector BoundMinRange = FVector(-GroundSize.X * 0.48f, -GroundSize.Y * 0.48f, -GroundSize.Z);
	const FVector BoundMaxRange = FVector(GroundSize.X * 0.48f, GroundSize.Y * 0.48f, -100.0f);
	const FVector Center = (BoundMinRange + BoundMaxRange) * 0.5f;
	const FVector HalfSize = (BoundMaxRange - BoundMinRange) * 0.5f;

	BoundBox->SetBoxExtent(HalfSize);
	BoundBox->SetRelativeLocation(Center);
	BoundBox->bHiddenInGame = false;
}

void AVoxelGround::OnPlayerEnter(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ABACharacter* Character = Cast<ABACharacter>(OtherActor);
	if (IsValid(Character) == false || Character->IsLocallyControlled() == false)
		return;

	SkyAtmosphere = SkyAtmosphere.IsValid() == false ? GetSkyAtmosphere() : SkyAtmosphere;
	if (SkyAtmosphere.IsValid() == false)
		return;

	USkyAtmosphereComponent* SkyComp = SkyAtmosphere->GetComponentByClass<USkyAtmosphereComponent>();
	if (IsValid(SkyComp) == false)
		return;

	USceneCaptureComponent2D* SceneCapture2D = Character->GetGroundScannerSceneCapture();
	if (IsValid(SceneCapture2D) == true)
	{
		SceneCapture2D->ShowOnlyActors.AddUnique(this);
		SceneCapture2D->HideComponent(BoundBox);
	}

	OriginMieScatterScale = SkyComp->MieScatteringScale;
	OriginReighScatterScale = SkyComp->RayleighScatteringScale;
	SkyComp->SetMieScatteringScale(0.0f);
	SkyComp->SetRayleighScatteringScale(0.0f);

	Character->Server_StartRecordingPath();
}

void AVoxelGround::OnPlayerExit(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	ABACharacter* Character = Cast<ABACharacter>(OtherActor);
	if (IsValid(Character) == false || Character->IsLocallyControlled() == false)
		return;

	SkyAtmosphere = SkyAtmosphere.IsValid() == false ? GetSkyAtmosphere() : SkyAtmosphere;
	if (SkyAtmosphere.IsValid() == false)
		return;

	USkyAtmosphereComponent* SkyComp = SkyAtmosphere->GetComponentByClass<USkyAtmosphereComponent>();
	if (IsValid(SkyComp) == false)
		return;

	SkyComp->SetMieScatteringScale(OriginMieScatterScale);
	SkyComp->SetRayleighScatteringScale(OriginReighScatterScale);

	if (Character->GetIsReturning() == false)
	{
		Character->Server_StopRecordingPath();
		Character->Server_ResetPath();
	}
}

ASkyAtmosphere* AVoxelGround::GetSkyAtmosphere() const
{
	UWorld* World = GetWorld();
	if (IsValid(World) == false)
		return nullptr;

	for (TActorIterator<ASkyAtmosphere> It(World); It; ++It)
	{
		return *It;
	}
	return nullptr;
}
