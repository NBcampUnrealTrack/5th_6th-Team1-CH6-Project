#include "Framework/LobbyGameMode.h"
#include "Multiplayer/MultiplayerSubsystem.h"
#include "Player/BAPlayerController.h"
#include "Multiplayer/PlayerColorSubsystem.h"
#include "Player/BAPlayerState.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Framework/BAGameState.h"

ALobbyGameMode::ALobbyGameMode()
{
    bUseSeamlessTravel = true;
}

void ALobbyGameMode::BeginPlay()
{
    Super::BeginPlay();

    UMultiplayerSubsystem* MultiplayerSubsystem = GetGameInstance()->GetSubsystem<UMultiplayerSubsystem>();
    if (IsValid(MultiplayerSubsystem) == true)
    {
        MultiplayerSubsystem->StartSessionHeartBeat();
    }

    TWeakObjectPtr<ALobbyGameMode> WeakThis = TWeakObjectPtr(this);
    GetWorldTimerManager().SetTimerForNextTick(
        [WeakThis]()
        {
            if (WeakThis.IsValid() == false)
                return;

            WeakThis->CreateRoom();
        });
}

void ALobbyGameMode::HandleSeamlessTravelPlayer(AController*& C)
{
    Super::HandleSeamlessTravelPlayer(C);

    ABAPlayerController* PC = Cast<ABAPlayerController>(C);
    if (IsValid(PC) == false)
        return;

    PC->SetLevelType(ELevelType::Lobby);
}

void ALobbyGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    UMultiplayerSubsystem* MultiplayerSubsystem = GetGameInstance()->GetSubsystem<UMultiplayerSubsystem>();
    if (IsValid(MultiplayerSubsystem) == true)
    {
        MultiplayerSubsystem->StopSessionHeartBeat();
    }

    Super::EndPlay(EndPlayReason);
}

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);

    UpdateSessionParticipants();

    ABAPlayerState* PS = NewPlayer->GetPlayerState<ABAPlayerState>();
    if (IsValid(PS) == true)
    {
        UPlayerColorSubsystem* PlayerColorSubsystem = GetGameInstance()->GetSubsystem<UPlayerColorSubsystem>();
        if (IsValid(PlayerColorSubsystem) == true)
        {
            int32 NewColorIdx = PlayerColorSubsystem->GetColorIndex(PS->GetUniqueId());
            PS->SetPlayerColorIdx(NewColorIdx);
        }
    }

    for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
    {
        ABAPlayerController* PC = Cast<ABAPlayerController>(It->Get());
        if (IsValid(PC) == true)
        {
            PC->Client_SetVoiceChatUser();
        }
    }
}

void ALobbyGameMode::Logout(AController* Exiting)
{
    ABAPlayerState* PS = Exiting->GetPlayerState<ABAPlayerState>();
    if (IsValid(PS) == true)
    {
        UPlayerColorSubsystem* PlayerColorSubsystem = GetGameInstance()->GetSubsystem<UPlayerColorSubsystem>();
        if (IsValid(PlayerColorSubsystem) == true)
        {
            PlayerColorSubsystem->ReleaseColorIndex(PS->GetUniqueId());
        }
    }

    Super::Logout(Exiting);

    GetWorldTimerManager().SetTimerForNextTick(this, &ThisClass::UpdateSessionParticipants);
}

void ALobbyGameMode::UpdateSessionParticipants()
{
    UMultiplayerSubsystem* MultiplayerSubsystem = GetGameInstance()->GetSubsystem<UMultiplayerSubsystem>();
    if (IsValid(MultiplayerSubsystem) == true)
    {
        MultiplayerSubsystem->UpdateSessionParticipants();
    }
}

void ALobbyGameMode::CreateRoom()
{
    UMultiplayerSubsystem* MultiplayerSubsystem = GetGameInstance()->GetSubsystem<UMultiplayerSubsystem>();
    if (IsValid(MultiplayerSubsystem) == true)
    {
        // GameMode는 호스트만 가지므로, 별도의 필터링 X
        CreateRoomHandle = MultiplayerSubsystem->BindOnCreateSession(FOnCreateSession::FDelegate::CreateUObject(
            this, &ThisClass::OnCreateRoom));

        MultiplayerSubsystem->CreateSession();
    }
}

void ALobbyGameMode::OnCreateRoom(FName SessionName, bool bWasSuccessful)
{
    UMultiplayerSubsystem* MultiplayerSubsystem = GetGameInstance()->GetSubsystem<UMultiplayerSubsystem>();
    if (IsValid(MultiplayerSubsystem) == false)
        return;

    APlayerController* PC = GetGameInstance()->GetFirstLocalPlayerController();
    if (IsValid(PC) == true)
    {
        ABAPlayerState* PS = PC->GetPlayerState<ABAPlayerState>();
        if (IsValid(PS) == true)
        {
            MultiplayerSubsystem->SyncNicknameToPlayerState(PS);
        }
    }

    MultiplayerSubsystem->UnbindOnCreateSession(CreateRoomHandle);
}
