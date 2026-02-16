#include "Framework/BAGameState.h"
#include "Mining/VoxelData.h"
#include "Kismet/KismetSystemLibrary.h"

void ABAGameState::SetOreCount(EVoxelType OreType, int32 Count)
{
	Multicast_UpdateOreCount(OreType, Count);
}

int32 ABAGameState::GetOreCount(EVoxelType OreType)
{
	const int32* OreCountPtr = OreInventory.Find(OreType);
	return OreCountPtr != nullptr ? *OreCountPtr : 0;
}

void ABAGameState::BindOnOreChanged(const FOnOreChanged::FDelegate& Delegate)
{
	OnOreChanged.Add(Delegate);
}

void ABAGameState::UnbindOnOreChanged(const UObject* Object)
{
	OnOreChanged.RemoveAll(Object);
}

void ABAGameState::Multicast_UpdateOreCount_Implementation(EVoxelType OreType, int32 Count)
{
	OreInventory.FindOrAdd(OreType) = Count;
	//UKismetSystemLibrary::PrintString(GetWorld(), *FString::FromInt(Count));

	OnOreChanged.Broadcast(OreType, Count);
}
