#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Mining/VoxelData.h"
#include "VoxelGround.generated.h"

class UVoxelGroundChunk;
struct FNeighborLOD;

UENUM()
enum class EChunkState
{
	Ground,
	Air,
	Complex
};

USTRUCT()
struct FVoxelGroundChunkData
{
	GENERATED_BODY()

	UPROPERTY()
	EChunkState ChunkState = EChunkState::Ground;
	UPROPERTY()
	TArray<uint8> DensityValues;
	UPROPERTY()
	int32 LODLevel = 0;					// 0이 가장 정밀한 LOD
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
	void DigGround(const FVector& WorldLocation, float Radius);
	void DigGround(int32 ChunkIdx, const FVector& ChunkOffset, const FVector& WorldLocation, float Radius);

	void EnqueueChunkUpdateResult(FChunkUpdateResult&& Result);

protected:
	void InitializeGround();
	void InitializeChunkData(int32 ChunkIdx);
	void SpawnChunk(int32 ChunkIdx);

	FVector GetPlayerLocation();					// 나중에 팀원들 전체 위치 가져오도록 수정
	void CheckPlayerChunk(const FVector& PlayerLocation);
	void UpdateNearByChunks(const FIntVector& PlayerChunkCoord);
	UFUNCTION()
	void UpdateChunkLODs();

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

#pragma region GroundSetting

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GroundSetting")
	FVector GroundSize = FVector(20000.0f, 20000.0f, 30000.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GroundSetting")
	int32 ChunkGridSize = 16;			// 복셀 갯수 - 청크의 한 변에 있는 복셀의 갯수
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GroundSetting")
	float ChunkVoxelSize = 100.0f;		// 복셀 간격
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GroundSetting")
	uint8 IsoLevel = 127;				// 지표면 임계값

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GroundSetting")
	float CaveScale = 0.0005f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GroundSetting")
	float CaveThreshold = 0.6f;

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

	FIntVector ChunkRangeMin;
	FIntVector ChunkRangeMax;
	FIntVector GridWidth;

	FIntVector LastPlayerChunkCoord;
	FTimerHandle UpdateChunkLODTimerHandle;

	TSet<int32> LastNearByChunkIdxs;
	uint8 bUpdateHighResNext : 1 = true;			// 플레이어 위치 기반으로 LOD 변경할 때, (고해상도 => 저해상도) / (저해상도 => 고해상도) 번갈아가면서 검사

	FTimerHandle UpdateDirtyChunkTimerHandle;
	TBitArray<> ChunkMeshDirties;
	uint8 bIsDirty : 1 = false;
	TBitArray<> PriorityChunkMeshDirties;
	uint8 bIsPriorityDirty : 1 = false;

	TQueue<FChunkUpdateResult, EQueueMode::Mpsc> ChunkUpdateResultQueue;
	int32 NextChunkUpdateID = 0;
	const int32 MaxUpdatePerFrame = 20;

#pragma endregion
};
