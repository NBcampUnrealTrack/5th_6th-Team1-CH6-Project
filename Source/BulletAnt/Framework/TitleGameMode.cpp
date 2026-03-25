#include "Framework/TitleGameMode.h"
#include "Multiplayer/MultiplayerSubsystem.h"
#include "Player/TitlePlayerController.h"

ATitleGameMode::ATitleGameMode()
{
}

void ATitleGameMode::BeginPlay()
{
	if (bAlreadyLogin == false)
	{
		StartLogin();
	}
}

void ATitleGameMode::StartLogin()
{
	UMultiplayerSubsystem* MultiplayerSubsystem = GetGameInstance()->GetSubsystem<UMultiplayerSubsystem>();
	if (IsValid(MultiplayerSubsystem) == true)
	{
		MultiplayerSubsystem->BindOnSuccessLogin(FOnSuccessLogin::FDelegate::CreateUObject(this, &ThisClass::OnSuccessLogin));
		MultiplayerSubsystem->SteamLogin();
	}
}

void ATitleGameMode::OnSuccessLogin()
{
	bAlreadyLogin = true;

	ATitlePlayerController* PC = GetWorld()->GetFirstPlayerController<ATitlePlayerController>();
	if (IsValid(PC) == true)
	{
		PC->ShowLoginPanel(false);
	}
}
