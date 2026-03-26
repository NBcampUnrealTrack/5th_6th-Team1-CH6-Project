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

    TWeakObjectPtr<ALobbyGameMode> WeakThis = TWeakObjectPtr(this);
    GetWorldTimerManager().SetTimerForNextTick(
        [WeakThis]()
        {
            if (WeakThis.IsValid() == false)
                return;

            WeakThis->CreateRoom();
        });

    // 테스트용 임시
    FTimerHandle TravelHandle;
    GetWorldTimerManager().SetTimer(
        TravelHandle,
        [WeakThis]()
        {
            if (WeakThis.IsValid() == false)
                return;

            UMultiplayerSubsystem* MultiplayerSubsystem = WeakThis->GetGameInstance()->GetSubsystem<UMultiplayerSubsystem>();
            if (IsValid(MultiplayerSubsystem) == true)
            {
                // GameMode는 호스트만 가지므로, 별도의 필터링 X
                MultiplayerSubsystem->ServerTravelToLevel("/Game/SpaceBase/Maps/MainLevel");
            }
        },
        60.0f,
        false);
}

void ALobbyGameMode::HandleSeamlessTravelPlayer(AController*& C)
{
    Super::HandleSeamlessTravelPlayer(C);

    ABAPlayerController* PC = Cast<ABAPlayerController>(C);
    if (IsValid(PC) == false)
        return;

    PC->SetLevelType(ELevelType::Lobby);
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
        AGameStateBase* GS = GetWorld()->GetGameState();
        if (IsValid(GS) == true)
        {
            const auto& PlayerArray = GS->PlayerArray;
            TArray<FString> PlayerNames;
            for (APlayerState* PS : PlayerArray)
            {
                if (IsValid(PS) == true)
                {
                    PlayerNames.Add(PS->GetPlayerName());
                }
            }

            MultiplayerSubsystem->UpdateSessionParticipants(PlayerNames);
        }
    }
}

void ALobbyGameMode::CreateRoom()
{
    UMultiplayerSubsystem* MultiplayerSubsystem = GetGameInstance()->GetSubsystem<UMultiplayerSubsystem>();
    if (IsValid(MultiplayerSubsystem) == true)
    {
        // GameMode는 호스트만 가지므로, 별도의 필터링 X
        UpdateParticipantsHandle = MultiplayerSubsystem->BindOnCreateSession(FOnCreateSession::FDelegate::CreateLambda(
            [MultiplayerSubsystem, WeakThis = TWeakObjectPtr(this)](FName, bool)
            {
                if (WeakThis.IsValid() == true)
                {
                    WeakThis->UpdateSessionParticipants();
                    MultiplayerSubsystem->UnbindOnCreateSession(WeakThis->UpdateParticipantsHandle);
                }
            }));

        MultiplayerSubsystem->CreateSession();
    }
}
