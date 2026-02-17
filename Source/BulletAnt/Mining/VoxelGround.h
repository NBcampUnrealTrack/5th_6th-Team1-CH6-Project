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
	UPROPERTY()
	TArray<uint8> DensityValues;						// 200 초과: 기반암
	UPROPERTY()
	TArray<EVoxelType> VoxelTypes;
	UPROPERTY()
	int32 LODLevel = 0;									// 0이 가장 정밀한 LOD

	UPROPERTY()
	int32 GroundVoxelCount = 0;
};

// 정정ㅁ Density 변경 후 반환할 데이터
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

USTRUCT()
struct FVoxelPointEditData
{
	GENERATED_BODY()

	UPROPERTY()
	int32 VoxelIndex = 0;
	UPROPERTY()
	uint8 NewDensityValue = 0;
};

USTRUCT()
struct FVoxelChunkEditData
{
	GENERATED_BODY()

	UPROPERTY()
	int32 ChunkIdx = -1;
	UPROPERTY()
	TArray<FVoxelPointEditData> PointEditDatas;

	UPROPERTY()
	int32 SendIdx = 0;			// Queue에서 전달할 때 사용
};

USTRUCT()
struct FVoxelChunkEditPacket
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FVoxelChunkEditData> ChunkEditDatas;
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
	bool DigGround(int32 ChunkIdx, const FVector& ChunkOffset, const FVector& WorldLocation, float Radius, FVoxelChunkEditData& OutData, TMap<EVoxelType, int32>& MinedOreMap);

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

	FIntVector ChunkRangeMin;
	FIntVector ChunkRangeMax;
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

	float OriginReighScatterScale = 0.0f;
	
#pragma endregion

};
