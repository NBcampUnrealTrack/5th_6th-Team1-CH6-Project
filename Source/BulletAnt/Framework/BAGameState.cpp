#include "Framework/BAGameState.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Building/BaseCore.h"
#include "Player/BAPlayerController.h"
#include "Mining/VoxelGroundSubsystem.h"
#include "Net/UnrealNetwork.h"
#include "Weapon/BaseWeapon.h"
#include "Player/BACharacter.h"

ABAGameState::ABAGameState()
{
    bReplicates = true;
}

void ABAGameState::BeginPlay()
{
    Super::BeginPlay();

    if (HasAuthority() == true)
    {
        GroundInitParams.Seed = FMath::RandRange(0, 56928);
        GroundInitParams.GroundType = EGroundType::Default;

        OnRep_SetInitParams();

        if (InitWeaponArray.Num() > 0)
        {
            for (TSubclassOf<ABaseWeapon> Weapon : InitWeaponArray)
            {
                AddHaveWeapon(Weapon);
            }
        }
    }
}

void ABAGameState::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ThisClass, GroundInitParams);
    DOREPLIFETIME(ThisClass, WavePreparationTime);
    DOREPLIFETIME(ThisClass, HaveWeaponArray);
}

void ABAGameState::AddActiveCharacter(ABACharacter* InCharacter)
{
    ActiveCharacters.Add(InCharacter);
}

void ABAGameState::RemoveActiveCharacter(ABACharacter* InCharacter)
{
    ActiveCharacters.Remove(InCharacter);
}

void ABAGameState::OnRep_SetInitParams()
{
    UVoxelGroundSubsystem* GroundSubsystem = GetWorld()->GetSubsystem<UVoxelGroundSubsystem>();
    ensureMsgf(IsValid(GroundSubsystem) == true, TEXT("VoxelGroundSubststem is not valid"));

    GroundSubsystem->CreateVoxelGround(GroundInitParams);
}

void ABAGameState::Multicast_EditGround_Implementation(const FVoxelChunkEditPacket& Packet)
{
    if (HasAuthority() == true)
        return;

    UVoxelGroundSubsystem* GroundSubsystem = GetWorld()->GetSubsystem<UVoxelGroundSubsystem>();
    ensureMsgf(IsValid(GroundSubsystem) == true, TEXT("VoxelGroundSubststem is not valid"));

    GroundSubsystem->ApplyEditPacket(Packet);
}

void ABAGameState::SetOreCount(EOreType OreType, int32 Count)
{
	Multicast_UpdateOreCount(OreType, Count);
}

int32 ABAGameState::GetOreCount(EOreType OreType)
{
	const int32* OreCountPtr = OreInventory.Find(OreType);
	return OreCountPtr != nullptr ? *OreCountPtr : 0;
}

bool ABAGameState::CanPurchase(const TMap<EOreType, int32>& Cost)
{
    if (!IsValid(this))
    {
        return false;
    }

    if (Cost.Num() == 0)
    {
        return true;
    }

    // 보유 광물 체크
    for (const TPair<EOreType, int32>& Pair : Cost)
    {
        const EOreType Type = Pair.Key;
        const int32 Need = Pair.Value;

        if (Need <= 0)
        {
            continue;
        }

        const int32 CurrOreCount = GetOreCount(Type);
        if (CurrOreCount < Need)
        {
            return false;
        }
    }

    return true;
}

void ABAGameState::BindOnOreChanged(const FOnOreChanged::FDelegate& Delegate)
{
	OnOreChanged.Add(Delegate);
}

void ABAGameState::UnbindOnOreChanged(const UObject* Object)
{
	OnOreChanged.RemoveAll(Object);
}

void ABAGameState::Multicast_UpdateOreCount_Implementation(EOreType OreType, int32 Count)
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

int32 ABAGameState::GetInitWavePreparationTime() const
{
    return InitWavePreparationTime;
}

void ABAGameState::SetInitWavePreparationTime(int32 InTime)
{
    InitWavePreparationTime = InTime;
}

int32 ABAGameState::GetWavePreparationTime() const
{
    return WavePreparationTime;
}

void ABAGameState::SetWavePreparationTime(int32 InTime)
{
    WavePreparationTime = InTime;
}

void ABAGameState::OnRep_WavePreparationTime()
{
    OnWaveTimeChanged.Broadcast();
}

void ABAGameState::AddHaveWeapon(TSubclassOf<ABaseWeapon> InWeaponClass)
{
    if (!HasAuthority()) return;

    if (!HaveWeaponArray.Contains(InWeaponClass))
    {
        HaveWeaponArray.Add(InWeaponClass);
    }
}
