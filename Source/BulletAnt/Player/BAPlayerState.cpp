#include "Player/BAPlayerState.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/KismetSystemLibrary.h"

void ABAPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, PlayerColorIdx);
}

void ABAPlayerState::CopyProperties(APlayerState* NewPlayerState)
{
	Super::CopyProperties(NewPlayerState);

	ABAPlayerState* NewPS = Cast<ABAPlayerState>(NewPlayerState);
	if (IsValid(NewPS) == false)
		return;

	NewPS->PlayerColorIdx = this->PlayerColorIdx;
}

void ABAPlayerState::SetPlayerColorIdx(int32 NewIdx)
{
	PlayerColorIdx = NewIdx;
	OnRep_PlayerColorIdx();
}

FLinearColor ABAPlayerState::GetPlayerColor() const
{
	return PlayerColorTable.IsValidIndex(PlayerColorIdx) == true ? PlayerColorTable[PlayerColorIdx] : FLinearColor::White;
}

FDelegateHandle ABAPlayerState::BindOnChangedPlayerColor(const FOnChangedPlayerColor::FDelegate& Delegate)
{
	return OnChangedPlayerColor.Add(Delegate);
}

void ABAPlayerState::UnbindOnChangedPlayerColor(const UObject* Object)
{
	OnChangedPlayerColor.RemoveAll(Object);
}

void ABAPlayerState::UnbindOnChangedPlayerColor(FDelegateHandle Handle)
{
	OnChangedPlayerColor.Remove(Handle);
}

void ABAPlayerState::OnRep_PlayerColorIdx()
{
	OnChangedPlayerColor.Broadcast(GetPlayerColor());
}
