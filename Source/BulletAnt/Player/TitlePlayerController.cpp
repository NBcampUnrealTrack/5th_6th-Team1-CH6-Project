

#include "Player/TitlePlayerController.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "UI/UISubsystem.h"
#include "UI/UW_TitleScreen.h"
#include "Multiplayer/MultiplayerSubsystem.h"

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

				FInputModeUIOnly Mode;
				SetInputMode(Mode);
				bShowMouseCursor = true;
			}
		}
	}
}

void ATitlePlayerController::HandleJoinRequested(const FText& InIpPort)
{
	UMultiplayerSubsystem* MultiplayerSubSystem = GetGameInstance()->GetSubsystem<UMultiplayerSubsystem>();
	if (IsValid(MultiplayerSubSystem) == true)
	{
		MultiplayerSubSystem->SearchSessions();
	}
}

void ATitlePlayerController::HandleOptionRequested()
{
	UMultiplayerSubsystem* MultiplayerSubSystem = GetGameInstance()->GetSubsystem<UMultiplayerSubsystem>();
	if (IsValid(MultiplayerSubSystem) == true)
	{
		MultiplayerSubSystem->CreateSession();
	}
}
