#include "Framework/BAGameInstance.h"
#include "Multiplayer/MultiplayerSubsystem.h"

void UBAGameInstance::OnStart()
{
	Super::OnStart();

	if (bLoginOnStart == true)
	{
		UMultiplayerSubsystem* MultiplayerSubsystem = GetSubsystem<UMultiplayerSubsystem>();
		if (IsValid(MultiplayerSubsystem) == true)
		{
			MultiplayerSubsystem->EpicLogin();
		}
	}
}
