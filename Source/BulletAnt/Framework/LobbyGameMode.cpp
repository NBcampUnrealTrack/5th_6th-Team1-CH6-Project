#include "Framework/LobbyGameMode.h"
#include "Multiplayer/MultiplayerSubsystem.h"
#include "Player/BAPlayerController.h"

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

            UMultiplayerSubsystem* MultiplayerSubsystem = WeakThis->GetGameInstance()->GetSubsystem<UMultiplayerSubsystem>();
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
