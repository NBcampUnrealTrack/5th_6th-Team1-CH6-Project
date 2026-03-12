#include "Framework/LobbyGameMode.h"
#include "Multiplayer/MultiplayerSubsystem.h"

ALobbyGameMode::ALobbyGameMode()
{
    bUseSeamlessTravel = true;
}

void ALobbyGameMode::BeginPlay()
{
    Super::BeginPlay();

    GetWorldTimerManager().SetTimerForNextTick(
        [this]()
        {
            UMultiplayerSubsystem* MultiplayerSubsystem = GetGameInstance()->GetSubsystem<UMultiplayerSubsystem>();
            if (IsValid(MultiplayerSubsystem) == true)
            {
                // GameMode는 호스트만 가지므로, 별도의 필터링 X
                MultiplayerSubsystem->CreateSession();
            }
        });

    // 테스트용 임시
    FTimerHandle TravelHandle;
    GetWorldTimerManager().SetTimer(
        TravelHandle,
        [this]()
        {
            UMultiplayerSubsystem* MultiplayerSubsystem = GetGameInstance()->GetSubsystem<UMultiplayerSubsystem>();
            if (IsValid(MultiplayerSubsystem) == true)
            {
                // GameMode는 호스트만 가지므로, 별도의 필터링 X
                MultiplayerSubsystem->ServerTravelToLevel("/Game/SpaceBase/Maps/MainLevel");
            }
        },
        60.0f,
        false);
}
