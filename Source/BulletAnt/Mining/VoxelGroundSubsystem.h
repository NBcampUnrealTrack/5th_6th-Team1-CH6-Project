#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Mining/VoxelData.h"
#include "VoxelGroundSubsystem.generated.h"

class AVoxelGround;
enum class EGroundType : uint8;
class UGroundSettingTable;

UCLASS()
class BULLETANT_API UVoxelGroundSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
	
public:
	UVoxelGroundSubsystem();

	void CreateVoxelGround(const FGroundInitializeParams& GroundInitParams, const FVector& Origin = FVector::ZeroVector);

	// 서버 전용
	void EnqueueEditData(const FVoxelChunkEditData& Data);

	// 클라 전용
	void ApplyEditPacket(const FVoxelChunkEditPacket& Packet);

private:
	void ProcessSendEditDataNextFrame();
	void OnProcessSendEditData();

protected:
	UPROPERTY()
	TObjectPtr<AVoxelGround> VoxelGround;
	UPROPERTY()
	TWeakObjectPtr<const UGroundSettingTable> SettingTable;

	// 서버 전용 - 서버에서 지형 변경, 변경 데이터 클라이언트로 순차적으로 전송
	TQueue<FVoxelChunkEditData> EditDataQueue;
	uint8 bScheduledSendEditData : 1 = false;
};
