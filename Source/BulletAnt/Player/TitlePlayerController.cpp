

#include "Player/TitlePlayerController.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "UI/UISubsystem.h"
#include "UI/UW_TitleScreen.h"

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
