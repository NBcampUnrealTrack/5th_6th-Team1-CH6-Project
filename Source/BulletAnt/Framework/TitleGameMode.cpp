#include "Framework/TitleGameMode.h"
#include "Multiplayer/MultiplayerSubsystem.h"
#include "Player/TitlePlayerController.h"

ATitleGameMode::ATitleGameMode()
{
}

void ATitleGameMode::BeginPlay()
{
	StartLogin();
}

void ATitleGameMode::StartLogin()
{
	UMultiplayerSubsystem* MultiplayerSubsystem = GetGameInstance()->GetSubsystem<UMultiplayerSubsystem>();
	if (IsValid(MultiplayerSubsystem) == true)
	{
		if (MultiplayerSubsystem->IsLogin() == true)
		{
			OnSuccessLogin();
			return;
		}

		MultiplayerSubsystem->BindOnSuccessLogin(FOnSuccessLogin::FDelegate::CreateUObject(this, &ThisClass::OnSuccessLogin));
		MultiplayerSubsystem->Login();
	}
}

void ATitleGameMode::OnSuccessLogin()
{
	ATitlePlayerController* PC = GetWorld()->GetFirstPlayerController<ATitlePlayerController>();
	if (IsValid(PC) == true)
	{
		PC->ShowLoginPanel(false);
	}
}
