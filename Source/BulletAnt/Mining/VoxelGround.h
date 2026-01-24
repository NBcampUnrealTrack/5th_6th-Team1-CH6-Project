#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VoxelGround.generated.h"

class UVoxelGroundChunk;

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
};

UCLASS()
class BULLETANT_API AVoxelGround : public AActor
{
	GENERATED_BODY()
	
public:	
	AVoxelGround();

protected:
	virtual void BeginPlay() override;

public:
	void DigGround(const FVector& WorldLocation, float Radius);
	void DigGround(int32 ChunkIdx, const FVector& ChunkOffset, const FVector& WorldLocation, float Radius);

protected:
	void InitializeGround();
	void InitializeDensityPerChunk(int32 ChunkIdx);
	void SpawnChunk(int32 ChunkIdx);

	int32 GetChunkIndex(int32 X, int32 Y, int32 Z) const;
	FIntVector GetChunkCoord(int32 ChunkIdx) const;
	FVector GetChunkOffset(int32 X, int32 Y, int32 Z) const;

#pragma region GroundSetting

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GroundSetting")
	FVector GroundSize = FVector(20000.0f, 20000.0f, 30000.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GroundSetting")
	int32 ChunkGridSize = 16;			// 복셀 갯수 - 청크의 한 변에 있는 복셀의 갯수
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GroundSetting")
	float ChunkVoxelSize = 100.0f;		// 복셀 간격
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GroundSetting")
	uint8 IsoLevel = 100;				// 지표면 임계값

#pragma endregion

#pragma region Props

	UPROPERTY()
	TArray<FVoxelGroundChunkData> ChunkDatas;
	UPROPERTY()
	TArray<TObjectPtr<UVoxelGroundChunk>> Chunks;

	FIntVector ChunkRangeMin;
	FIntVector ChunkRangeMax;
	FIntVector GridWidth;

#pragma endregion
};
