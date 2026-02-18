#include "Mining/VoxelGroundSubsystem.h"
#include "Mining/VoxelGround.h"
#include "Framework/BAGameState.h"
#include "Mining/GroundSettingTable.h"
#include "Mining/GroundSettingPreset.h"

UVoxelGroundSubsystem::UVoxelGroundSubsystem()
{
	static ConstructorHelpers::FObjectFinder<UGroundSettingTable> Table(TEXT("/Game/BulletAnt/Mining/GroundSetting/DA_GroundSettingTable.DA_GroundSettingTable"));
	SettingTable = Table.Object;
}

void UVoxelGroundSubsystem::CreateVoxelGround(const FGroundInitializeParams& GroundInitParams, const FVector& Origin)
{
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	VoxelGround = GetWorld()->SpawnActor<AVoxelGround>(
		AVoxelGround::StaticClass(),
		Origin,
		FRotator::ZeroRotator,
		SpawnParams);

	ensureMsgf(IsValid(VoxelGround) == true, TEXT("Voxel Ground is not created"));

	const UGroundSettingPreset* Setting = nullptr;
	if (SettingTable.IsValid() == true)
	{
		if (const TObjectPtr<const UGroundSettingPreset>* Ptr = SettingTable->Settings.Find(GroundInitParams.GroundType))
		{
			Setting = *Ptr;
		}
	}
	ensureMsgf(IsValid(Setting) == true, TEXT("Ground Setting is not valid"));

	VoxelGround->InitializeGround(GroundInitParams.Seed, Setting);
}

void UVoxelGroundSubsystem::EnqueueEditData(const FVoxelChunkEditData& Data)
{
	if (GetWorld()->GetNetMode() != NM_ListenServer)
		return;

	EditDataQueue.Enqueue(Data);
	ProcessSendEditDataNextFrame();
}

void UVoxelGroundSubsystem::ApplyEditPacket(const FVoxelChunkEditPacket& Packet)
{
	if (GetWorld()->GetNetMode() != NM_Client || IsValid(VoxelGround) == false)
		return;

	VoxelGround->ApplyEditPacket(Packet);
}

void UVoxelGroundSubsystem::ProcessSendEditDataNextFrame()
{
	if (bScheduledSendEditData == true)
		return;

	bScheduledSendEditData = true;
	GetWorld()->GetTimerManager().SetTimerForNextTick(
		this,
		&ThisClass::OnProcessSendEditData);
}

void UVoxelGroundSubsystem::OnProcessSendEditData()
{
	bScheduledSendEditData = false;

	int32 SyncLimit = 100;
	int32 SyncCount = 0;
	FVoxelChunkEditPacket EditData;
	while (EditDataQueue.IsEmpty() == false && SyncCount < SyncLimit)
	{
		FVoxelChunkEditData* QueueData = EditDataQueue.Peek();
		if (QueueData->SendIdx >= QueueData->PointEditDatas.Num())
		{
			EditDataQueue.Pop();
			continue;
		}

		int32 RemainLimit = SyncLimit - SyncCount;
		int32 RemainData = QueueData->PointEditDatas.Num() - QueueData->SendIdx;
		int32 Count = FMath::Min(RemainLimit, RemainData);

		FVoxelChunkEditData SendData;
		SendData.ChunkIdx = QueueData->ChunkIdx;
		SendData.PointEditDatas.SetNum(Count);
		for (int32 Idx = 0; Idx < Count; ++Idx)
		{
			SendData.PointEditDatas[Idx] = QueueData->PointEditDatas[QueueData->SendIdx + Idx];
		}
		EditData.ChunkEditDatas.Add(MoveTemp(SendData));

		QueueData->SendIdx += Count;
		SyncCount += Count;
	}

	if (SyncCount > 0)
	{
		ABAGameState* GS = GetWorld()->GetGameState<ABAGameState>();
		ensureMsgf(IsValid(GS) == true, TEXT("GameState is not valid"));

		GS->Multicast_EditGround(EditData);
	}

	if (EditDataQueue.IsEmpty() == false)
	{
		ProcessSendEditDataNextFrame();
	}
}
