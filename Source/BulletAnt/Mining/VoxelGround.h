#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Mining/VoxelData.h"
#include "VoxelGround.generated.h"

class UVoxelGroundChunk;
struct FNeighborLOD;
class ABAPlayerController;
class UGroundSettingPreset;

UENUM()
enum class EChunkState
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
	UPROPERTY()
	TArray<uint8> DensityValues;						// 200 초과: 기반암
	UPROPERTY()
	int32 LODLevel = 0;									// 0이 가장 정밀한 LOD

	UPROPERTY()
	int32 GroundVoxelCount = 0;
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

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	void DigGround(const FVector& WorldLocation, float Radius);
	bool DigGround(int32 ChunkIdx, const FVector& ChunkOffset, const FVector& WorldLocation, float Radius, FVoxelChunkEditData& OutData);

	bool MakeChunkSaveData(int32 ChunkIdx, FVoxelGroundChunkSaveData& OutData);
	bool LoadChunkSaveData(const FVoxelGroundChunkSaveData& Data);

	void EnqueueChunkUpdateResult(FChunkUpdateResult&& Result);

protected:
	void InitializeGround();
	void InitializeChunkData(int32 ChunkIdx);
	void InitializeChunkDensities(int32 ChunkIdx);
	void SpawnChunk(int32 ChunkIdx);

	void UpdateNearByChunks(const TArray<FIntVector>& PlayerChunkCoords);
	UFUNCTION()
	void UpdateChunkLODs();

	bool ChangeChunkDensityValue(int32 ChunkIdx, int32 PointIdx, int32 NewDensityValue);
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

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_EditGround(const FVoxelChunkEditPacket& Packet);
	void EditGroundChunk(const FVoxelChunkEditData& Data);
	void EnqueueChunkEditData(const FVoxelChunkEditData& Data);

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

	UPROPERTY(Replicated)
	FIntVector ChunkRangeMin;
	UPROPERTY(Replicated)
	FIntVector ChunkRangeMax;
	UPROPERTY(Replicated)
	FIntVector GridWidth;

	FTimerHandle UpdateChunkLODTimerHandle;
	TSet<int32> LastNearByChunkIdxs;

	FTimerHandle UpdateDirtyChunkTimerHandle;
	TBitArray<> ChunkMeshDirties;
	uint8 bIsDirty : 1 = false;
	TBitArray<> PriorityChunkMeshDirties;
	uint8 bIsPriorityDirty : 1 = false;

	TQueue<FChunkUpdateResult, EQueueMode::Mpsc> ChunkUpdateResultQueue;
	int32 NextChunkUpdateID = 0;
	const int32 MaxUpdatePerFrame = 20;

	TQueue<FVoxelChunkEditData> EditDataQueue;

#pragma endregion
};
