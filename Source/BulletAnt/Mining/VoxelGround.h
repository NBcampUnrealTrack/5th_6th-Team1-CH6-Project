#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Mining/VoxelData.h"
#include "VoxelGround.generated.h"

class UVoxelGroundChunk;
struct FNeighborLOD;
class ABAPlayerController;
class UGroundSettingPreset;
class UBoxComponent;
enum class EGroundType : uint8;

UENUM()
enum class EChunkState : uint8
{
	Ground,								// 기반암 없는 땅만
	Air,								// 공기만
	Complex								// 땅 + 공기 or 기반암이 섞인 땅
};

USTRUCT()
struct FVoxelGroundChunkData
{
	GENERATED_BODY()

	UPROPERTY()
	EChunkState ChunkState = EChunkState::Ground;

	TSharedRef<TArray<uint8>> DensityValues{};
	TSharedRef<TArray<EVoxelType>> VoxelTypes{};

	UPROPERTY()
	int32 LODLevel = 0;									// 0이 가장 정밀한 LOD

	UPROPERTY()
	int32 GroundVoxelCount = 0;

	FVoxelGroundChunkData()
		: DensityValues(MakeShared<TArray<uint8>>()), VoxelTypes(MakeShared<TArray<EVoxelType>>()) {}
};

// 정점 Density 변경 후 반환할 데이터
USTRUCT()
struct FVoxelChangedResult
{
	GENERATED_BODY()

	UPROPERTY()
	EVoxelType PrevType = EVoxelType::None;
	UPROPERTY()
	EVoxelType CurrType = EVoxelType::None;
	UPROPERTY()
	uint8 PrevDensity = 0;
	UPROPERTY()
	uint8 CurrDensity = 0;

	UPROPERTY()
	uint8 bTypeChanged : 1 = false;
};

// 레벨 전환 시 데이터 저장용
USTRUCT()
struct FVoxelGroundChunkSaveData
{
	GENERATED_BODY()

	UPROPERTY()
	int32 ChunkIdx = -1;
	UPROPERTY()
	EChunkState ChunkState = EChunkState::Ground;
	UPROPERTY()
	TArray<uint8> CompressedDensityValues;
	UPROPERTY()
	int32 UncompressedSize = 0;
};

USTRUCT()
struct FBuriedLocationData
{
	GENERATED_BODY()

	UPROPERTY()
	FTransform BuriedTransform;
	UPROPERTY()
	TSubclassOf<AActor> BuriedActorClass;
	UPROPERTY()
	TArray<FBuryBoundInfo> BoundInfos;

	UPROPERTY()
	uint8 bCarveDensity : 1 = true;
};

USTRUCT()
struct FPillarLocationData
{
	GENERATED_BODY()

	UPROPERTY()
	FVector PillarLocation;
	UPROPERTY()
	float PillarRadius;
	UPROPERTY()
	float PillarHeight;
	UPROPERTY()
	FVector PillarNormal;
};

USTRUCT()
struct FVeinLocationData
{
	GENERATED_BODY()

	UPROPERTY()
	FVector VeinLocation;
	UPROPERTY()
	float VeinRadius;
};

UCLASS()
class BULLETANT_API AVoxelGround : public AActor
{
	GENERATED_BODY()
	
public:	
	AVoxelGround();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void Tick(float DeltaSeconds) override;

public:
	void InitializeGround(int32 InSeed, const UGroundSettingPreset* InSetting);

	void DigGround(TMap<EVoxelType, int32>& HitMap, TMap<EOreType, int32>& MinedOreMap, const FVector& WorldLocation, float Radius);
	bool DigGround(int32 ChunkIdx, const FVector& ChunkOffset, const FVector& WorldLocation, float Radius, FVoxelChunkEditData& OutData, TMap<EVoxelType, int32>& HitMap, TMap<EOreType, int32>& MinedOreMap);

	bool MakeChunkSaveData(int32 ChunkIdx, FVoxelGroundChunkSaveData& OutData);
	bool LoadChunkSaveData(const FVoxelGroundChunkSaveData& Data);

	void EnqueueChunkUpdateResult(FChunkUpdateResult&& Result);

	void ApplyEditPacket(const FVoxelChunkEditPacket& Packet);

protected:
	void InitializeChunkData(int32 ChunkIdx);
	void InitializeChunkDensities(int32 ChunkIdx);
	void PlaceObjectsByCell();
	uint8 GetBaseDensity(const FVector& WorldPos);
	void SpawnChunk(int32 ChunkIdx);

	void UpdateNearByChunks(const TSet<FIntVector>& CoordsChanged);
	UFUNCTION()
	void UpdateChunkLODs();

	bool ChangeChunkDensityValue(int32 ChunkIdx, int32 PointIdx, int32 NewDensityValue, FVoxelChangedResult& OutResult);
	uint8 GetChunkDensityValue(int32 ChunkIdx, int32 PointIdx);

	int32 GetChunkIndex(int32 X, int32 Y, int32 Z) const;
	int32 GetChunkIndex(const FIntVector ChunkCoord) const;
	FIntVector GetChunkCoord(int32 ChunkIdx) const;
	FVector GetChunkOffset(int32 X, int32 Y, int32 Z) const;
	int32 GetLODLevelByPlayer(const FIntVector& ChunkCoord, const FIntVector& PlayerChunkCoord, int32 CurrentLODLevel);
	int32 GetChunkLODLevel(const FIntVector& ChunkCoord);
	int32 GetChunkLODLevel(int32 ChunkIdx);
	FNeighborLOD GetNeighborLOD(const FIntVector& ChunkCoord);
	FNeighborLOD GetNeighborLOD(int32 ChunkIdx);
	void AddChunkAndNeighbors(int32 ChunkIdx, TSet<int32>& ChunkIdxs);
	void UpdateChunkMesh(int32 ChunkIdx, bool bIncludeNeighbors);
	void UpdateChunkMeshImmediately(int32 ChunkIdx, bool bIncludeNeighbors);
	void UpdateDirtyChunks();
	void UpdatePriorityDirtyChunks();

	void EditGroundChunk(const FVoxelChunkEditData& Data);

#pragma region GroundSetting

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GroundSetting")
	TObjectPtr<const UGroundSettingPreset> Setting;

	TArray<int32> LODDistance = { 1, 2 };
	const int32 LODDistMargin = 1;				// 가까워질 때에는 LODDistance에 맞춰서 UpdateMesh, 멀어질 때에는 1만큼 더 여유 두고 UpdateMesh

	static const FIntVector NeighborOffsets[6];

#pragma endregion

#pragma region Props

protected:
	UPROPERTY()
	TArray<FVoxelGroundChunkData> ChunkDatas;
	UPROPERTY()
	TArray<TObjectPtr<UVoxelGroundChunk>> Chunks;

	int32 Seed = 0;

	FIntVector ChunkRangeMin;
	FIntVector ChunkRangeMax;
	FIntVector GridWidth;

	float ChunkSize = 0.0f;
	float BuriedRegionSize = 0.0f;
	float CellSize = 0.0f;
	TMap<FIntVector, FBuriedLocationData> BuriedLocationDatas;				// BuriedActor 영향권 청크
	TMap<FIntVector, FPillarLocationData> PillarLocationDatas;				// Pillar 광물 영향권 Cell
	TMap<FIntVector, FVeinLocationData> VeinLocationDatas;					// Vein 광물 영향권 Cell

	FTimerHandle UpdateChunkLODTimerHandle;
	TSet<FIntVector> LastPlayerCoords;

	FTimerHandle UpdateDirtyChunkTimerHandle;
	TBitArray<> ChunkMeshDirties;
	uint8 bIsDirty : 1 = false;
	TBitArray<> PriorityChunkMeshDirties;
	uint8 bIsPriorityDirty : 1 = false;

	TQueue<FChunkUpdateResult, EQueueMode::Mpsc> ChunkUpdateResultQueue;
	int32 NextChunkUpdateID = 0;
	const int32 MaxUpdatePerFrame = 20;

#pragma endregion

#pragma region InitialLoading
	
public:
	void OnInitializeComplete();
	
protected:
	UPROPERTY()
	int32 InitialChunkCount = 10000;
	UPROPERTY()
	uint8 bInitialLoading : 1 = false;
	UPROPERTY()
	TSet<int32> InitialChunkSet;

#pragma endregion

#pragma region Bound

protected:
	void SetBoundBox();

	UFUNCTION()
	void OnPlayerEnter(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	void OnPlayerExit(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	class ASkyAtmosphere* GetSkyAtmosphere() const;


protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UBoxComponent> BoundBox;
	UPROPERTY()
	TWeakObjectPtr<class ASkyAtmosphere> SkyAtmosphere;

	float OriginMieScatterScale = 0.0f;
	float OriginReighScatterScale = 0.0f;
	
#pragma endregion

};
