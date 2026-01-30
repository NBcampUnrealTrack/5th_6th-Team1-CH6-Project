#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
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

public:
	void DigGround(const FVector& WorldLocation, float Radius);
	void DigGround(int32 ChunkIdx, const FVector& ChunkOffset, const FVector& WorldLocation, float Radius);

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
	int32 GetLODLevelByPlayer(const FIntVector& ChunkCoord, const FIntVector& PlayerChunkCoord);
	int32 GetChunkLODLevel(const FIntVector& ChunkCoord);
	int32 GetChunkLODLevel(int32 ChunkIdx);
	FNeighborLOD GetNeighborLOD(const FIntVector& ChunkCoord);
	FNeighborLOD GetNeighborLOD(int32 ChunkIdx);
	void AddChunkAndNeighbors(int32 ChunkIdx, TSet<int32>& ChunkIdxs);

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

	TArray<int32> LODDistance = { 1, 3, 5 };

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

#pragma endregion
};
