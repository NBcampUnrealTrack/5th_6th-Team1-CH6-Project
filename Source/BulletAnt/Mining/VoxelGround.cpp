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

void AVoxelGround::DigGround(const FVector& WorldLocation, float Radius)
{
	if (GetNetMode() != NM_ListenServer)
		return;

	TRACE_CPUPROFILER_EVENT_SCOPE(DigGround);
	FVector RelativeLocation = WorldLocation - GetActorLocation();
	float ChunkSize = Setting->ChunkGridSize * Setting->ChunkVoxelSize;

	int32 MinX = FMath::FloorToInt((RelativeLocation.X - Radius) / ChunkSize);
	int32 MaxX = FMath::FloorToInt((RelativeLocation.X + Radius) / ChunkSize);
	int32 MinY = FMath::FloorToInt((RelativeLocation.Y - Radius) / ChunkSize);
	int32 MaxY = FMath::FloorToInt((RelativeLocation.Y + Radius) / ChunkSize);
	int32 MinZ = FMath::FloorToInt((RelativeLocation.Z - Radius) / ChunkSize);
	int32 MaxZ = FMath::FloorToInt((RelativeLocation.Z + Radius) / ChunkSize);

	UVoxelGroundSubsystem* GroundSubsystem = GetWorld()->GetSubsystem<UVoxelGroundSubsystem>();
	ensureMsgf(IsValid(GroundSubsystem) == true, TEXT("VoxelGroundSubststem is not valid"));

	TMap<EVoxelType, int32> MinedOreMap;
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
		if (Pair.Key == EVoxelType::BedRock ||
			Pair.Key == EVoxelType::NormalRock ||
			Pair.Key == EVoxelType::None)
			continue;

		ABAGameMode* GameMode = GetWorld()->GetAuthGameMode<ABAGameMode>();
		if (IsValid(GameMode) == true)
		{
			GameMode->MineOre(Pair.Key, Pair.Value);
		}
	}
}

bool AVoxelGround::DigGround(int32 ChunkIdx, const FVector& ChunkOffset, const FVector& WorldLocation, float Radius, FVoxelChunkEditData& OutData, TMap<EVoxelType, int32>& MinedOreMap)
{
	const FVector RelativeLocation = WorldLocation - GetActorLocation();
	const float ChunkSize = Setting->ChunkGridSize * Setting->ChunkVoxelSize;
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
								++MinedOreMap.FindOrAdd(OutResult.PrevType);
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

	float ChunkSize = Setting->ChunkGridSize * Setting->ChunkVoxelSize;

	const FVector& GroundSize = Setting->GroundSize;
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
	const float CaveScale = Setting->CaveScale;
	const float CaveThreshold = Setting->CaveThreshold;
	const uint8 IsoLevel = Setting->IsoLevel;
	const EVoxelType VeinVoxelType = Setting->VeinVoxelType;
	const EVoxelType PillarVoxelType = Setting->PillarVoxelType;

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

					// 임시 시드
					const int32 Seed = 1337;

					// 전체 지형 Bound - Bound 외부는 반드시 공기
					bool bInBound =
						WorldPos.X >= BoundMinRange.X && WorldPos.X <= BoundMaxRange.X &&
						WorldPos.Y >= BoundMinRange.Y && WorldPos.Y <= BoundMaxRange.Y &&
						WorldPos.Z >= BoundMinRange.Z && WorldPos.Z <= BoundMaxRange.Z;

					// 시드 기반으로 바꿔야 함. PerlinNoise 매개변수에 섞는 방법 고려.
					auto CalculateCave =
						[&](const FVector& WorldPos) -> uint8
						{
							float Density = 200.0f;

							float CaveValue = FMath::PerlinNoise3D(WorldPos * CaveScale);
							float CaveDensity = 100.0f + (CaveThreshold - CaveValue) * 200.0f;

							Density = FMath::Min(Density, CaveDensity);
							return (uint8)FMath::Clamp(FMath::RoundToInt(Density), 0, 200);
						};

					auto CalculateVein =
						[&](const FVector& WorldPos, const float ChunkVoxelSize)
						{
							float CellSize = 1600.0f;
							int32 CellX = FMath::FloorToInt(WorldPos.X / CellSize);
							int32 CellY = FMath::FloorToInt(WorldPos.Y / CellSize);
							int32 CellZ = FMath::FloorToInt(WorldPos.Z / CellSize);

							int32 Hash = HashCombine(HashCombine(CellX, CellY), HashCombine(CellZ, Seed));
							FRandomStream Stream(Hash);

							if (Stream.FRand() < 0.7f)
								return;

							// 400 ~ 560 / 800 ~ 1120
							float Radius = 400.0f + Stream.FRand() * 160.0f;
							// Cell의 양끝에서 Radius만큼은 떨어져야 옆 청크 침범 X
							float Available = CellSize - Radius * 2.0f;

							FVector VeinCenter(
								CellX * CellSize + Radius + (Available * Stream.FRand()),
								CellY * CellSize + Radius + (Available * Stream.FRand()),
								CellZ * CellSize + Radius + (Available * Stream.FRand()));

							float Dist = FVector::Distance(WorldPos, VeinCenter);

							if (Dist > Radius)
								return;

							float t = 1.0f - (Dist / Radius);

							float RoughNoise = FMath::PerlinNoise3D(WorldPos * 0.02f);
							float Roughness = RoughNoise * 4.0f;

							VoxelType = VeinVoxelType;
							FinalDensity = 200;// (uint8)FMath::Clamp(FinalDensity + 80, 0, 200);
						};

					auto CalculatePillar =
						[&](const FVector& WorldPos, const int32 PointIdx)
						{
							float CellSize = 1600.0f;
							int32 CellX = FMath::FloorToInt(WorldPos.X / CellSize);
							int32 CellY = FMath::FloorToInt(WorldPos.Y / CellSize);
							int32 CellZ = FMath::FloorToInt(WorldPos.Z / CellSize);

							int32 Hash = HashCombine(HashCombine(CellX, CellY), HashCombine(CellZ, Seed));
							FRandomStream Stream(Hash);

							if (Stream.FRand() > 0.5f)
								return;

							// 안정적으로 만들기 위해 일단 셀 중앙에서만 생성
							FVector PillarCenter(
								(CellX + 0.5f) * CellSize,
								(CellY + 0.5f) * CellSize,
								(CellZ + 0.5f) * CellSize);

							// 노이즈 기반으로 맵을 생성했기 때문에 지표면 근처는 Density가 IsoLevel에 가까움
							bool bIsNearSurface = FMath::Abs(CalculateCave(PillarCenter) - IsoLevel) < 8;
							if (bIsNearSurface == false)
								return;

							float Dx = CalculateCave(PillarCenter + FVector(ChunkVoxelSize, 0, 0)) - CalculateCave(PillarCenter - FVector(ChunkVoxelSize, 0, 0));
							float Dy = CalculateCave(PillarCenter + FVector(0, ChunkVoxelSize, 0)) - CalculateCave(PillarCenter - FVector(0, ChunkVoxelSize, 0));
							float Dz = CalculateCave(PillarCenter + FVector(0, 0, ChunkVoxelSize)) - CalculateCave(PillarCenter - FVector(0, 0, ChunkVoxelSize));

							// 지형에 Normal한 방향으로 기둥이 자라야 함. Density가 큰 쪽이 지형이므로 -연산.
							FVector Normal = -FVector(Dx, Dy, Dz).GetSafeNormal();
							FVector Delta = WorldPos - PillarCenter;
							float AlongNormal = FVector::DotProduct(Delta, Normal);
							float Radial = (Delta - (Normal * AlongNormal)).Length();
							float Radius = 240.f;
							float Height = 400.0f + 400.0f * Stream.FRand();

							if (AlongNormal < 0 || Radial > Radius)
								return;

							float Distance = Radial / Radius;
							float DistanceValue = (1.0f - Distance) * (1.0f - (AlongNormal / Height) * 0.2f);

							VoxelType = PillarVoxelType;
							FinalDensity = (uint8)FMath::Clamp(DistanceValue * 200, 0, 200);
						};

					// Bound 내부라면
					if (bInBound == true)
					{
						FVector LocalPos = WorldPos - Center;
						FVector AbsLocalPos = LocalPos.GetAbs();

						float MaxAxisRatio = FMath::Max3(
							AbsLocalPos.X / HalfSize.X,
							AbsLocalPos.Y / HalfSize.Y,
							AbsLocalPos.Z / HalfSize.Z);

						bool bBedRock = false;
						const static float BedRockNoiseWeight = 0.058f;
						const static float BedRockThickness = 0.06f;
						// MaxAxisRatio가 기반암이 생성 가능한 수치 이내라면, 위쪽 중앙 입구 지역이 아니라면
						if (MaxAxisRatio >= (1 - BedRockNoiseWeight - BedRockThickness) &&
							(WorldPos.Z <= Center.Z || AbsLocalPos.X > 400.0f || AbsLocalPos.Y > 400.0f))
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
							// 기반암이 아니라면, Cave 계산
							FinalDensity = CalculateCave(WorldPos);

							bool bGround = FinalDensity > IsoLevel;
							if (bGround == true)
							{
								VoxelType = EVoxelType::NormalRock;

								// 땅이라면, Vein 계산
								CalculateVein(WorldPos, ChunkVoxelSize);
							}
							else
							{
								// 공기라면, Pillar 계산
								CalculatePillar(WorldPos, PointIdx);
							}
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
		NewChunkState = GroundVoxelCount == 0 ? EChunkState::Ground : EChunkState::Air;
	}
	ChunkDatas[ChunkIdx].ChunkState = NewChunkState;

	ChunkDatas[ChunkIdx].GroundVoxelCount = GroundVoxelCount;
	ChunkDatas[ChunkIdx].DensityValues = MoveTemp(TempDensities);
	ChunkDatas[ChunkIdx].VoxelTypes = MoveTemp(TempVoxelTypes);
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

	float ChunkSize = Setting->ChunkGridSize * Setting->ChunkVoxelSize;

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
	if (PointIdx < 0 || PointIdx > TotalPoints)
		return false;

	bool bIsGroundNew = NewDensityValue > Setting->IsoLevel;
	if (ChunkDatas[ChunkIdx].DensityValues.IsEmpty() == true)
	{
		ensureMsgf(ChunkDatas[ChunkIdx].ChunkState == EChunkState::Complex, TEXT("Complex Chunk, but no density values"));
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
			ChunkDatas[ChunkIdx].ChunkState = ChunkDatas[ChunkIdx].GroundVoxelCount == 0 ? EChunkState::Ground : EChunkState::Air;
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
	float ChunkSize = Setting->ChunkGridSize * Setting->ChunkVoxelSize;
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

	OriginReighScatterScale = SkyComp->RayleighScatteringScale;
	//SkyComp->SetRayleighScatteringScale(0.0f);
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

	SkyComp->SetRayleighScatteringScale(OriginReighScatterScale);
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
