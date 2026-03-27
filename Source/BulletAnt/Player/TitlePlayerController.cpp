

#include "Player/TitlePlayerController.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "UI/UISubsystem.h"
#include "UI/UW_TitleScreen.h"
#include "Multiplayer/MultiplayerSubsystem.h"
#include "UI/UW_RoomList.h"
#include "UI/UW_CreateRoom.h"

ATitlePlayerController::ATitlePlayerController()
{
}

void ATitlePlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalController() == false)
	{
		return;
	}

	TitleScreenWidget = nullptr;

	if (auto* LP = GetLocalPlayer())
	{
		if (auto* UIS = LP->GetSubsystem<UUISubsystem>())
		{
			UISubsystem = UIS;
			TitleScreenWidget = UIS->ShowUI<UUW_TitleScreen>(EUIType::Title);
			if (IsValid(TitleScreenWidget))
			{
				TitleScreenWidget->OnJoinButtonClicked.AddDynamic(this, &ThisClass::HandleJoinRequested);
				TitleScreenWidget->OnOptionButtonClicked.AddDynamic(this, &ThisClass::HandleOptionRequested);

				TitleScreenWidget->ShowLoginPanel(bShowLoginPanel);

				FInputModeUIOnly Mode;
				SetInputMode(Mode);
				bShowMouseCursor = true;
			}
		}
	}
}

void ATitlePlayerController::ShowLoginPanel(bool bInShow)
{
	bShowLoginPanel = bInShow;

	if (IsValid(TitleScreenWidget) == true)
	{
		TitleScreenWidget->ShowLoginPanel(bShowLoginPanel);
	}
}

void ATitlePlayerController::HandleJoinRequested(const FText& InIpPort)
{
	ULocalPlayer* LP = GetLocalPlayer();
	if (IsValid(LP) == false)
		return;

	UUISubsystem* UIS = LP->GetSubsystem<UUISubsystem>();
	if (IsValid(UIS) == false)
		return;

	UUW_RoomList* RoomListUI = UIS->ShowUI<UUW_RoomList>(EUIType::RoomList);
	if (IsValid(RoomListUI) == false)
		return;

	RoomListUI->RefreshList();

	/*UMultiplayerSubsystem* MultiplayerSubSystem = GetGameInstance()->GetSubsystem<UMultiplayerSubsystem>();
	if (IsValid(MultiplayerSubSystem) == true)
	{
		MultiplayerSubSystem->SearchSessions();
	}*/
}

void ATitlePlayerController::HandleOptionRequested()
{
	ULocalPlayer* LP = GetLocalPlayer();
	if (IsValid(LP) == false)
		return;

	UUISubsystem* UIS = LP->GetSubsystem<UUISubsystem>();
	if (IsValid(UIS) == false)
		return;

	UUW_CreateRoom* CreateRoomUI = UIS->ShowUI<UUW_CreateRoom>(EUIType::CreateRoom);

	/*UMultiplayerSubsystem* MultiplayerSubSystem = GetGameInstance()->GetSubsystem<UMultiplayerSubsystem>();
	if (IsValid(MultiplayerSubSystem) == true)
	{
		MultiplayerSubSystem->ServerTravelToLobby();
	}*/
}
