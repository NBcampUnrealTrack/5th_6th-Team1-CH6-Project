#include "Framework/BAGameState.h"
#include "Mining/VoxelData.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Building/BaseCore.h"
#include "Player/BAPlayerController.h"

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

ABaseCore* ABAGameState::GetTargetCore() const
{
	return TargetCore;
}

void ABAGameState::SetTargetCore(ABaseCore* InTargetCore)
{
	TargetCore = InTargetCore;
}

void ABAGameState::AddPlayerController(ABAPlayerController* NewPlayer)
{
    if (IsValid(NewPlayer))
    {
        ConnectedPlayers.Add(NewPlayer);
    }
    //if (NewPlayer && !ConnectedPlayers.Contains(NewPlayer))
    //{
    //    ConnectedPlayers.Add(NewPlayer);
    //    UE_LOG(LogTemp, Log, TEXT("Player added to GameState. Total players: %d"), ConnectedPlayers.Num());
    //}
}

void ABAGameState::RemovePlayerController(ABAPlayerController* ExitingPlayer)
{
    if (IsValid(ExitingPlayer))
    {
        ConnectedPlayers.Remove(ExitingPlayer);
    }
}

TArray<ABAPlayerController*> ABAGameState::GetAllPlayerControllers() const
{
    return ConnectedPlayers;
}

ABAPlayerController* ABAGameState::GetPlayerControllerByIndex(int32 Index) const
{
    if (ConnectedPlayers.IsValidIndex(Index))
    {
        return ConnectedPlayers[Index];
    }
    return nullptr;
}