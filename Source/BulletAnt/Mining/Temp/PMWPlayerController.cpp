#include "Mining/Temp/PMWPlayerController.h"
#include "EnhancedInputSubsystems.h"

APMWPlayerController::APMWPlayerController() :
	IMC_Character(nullptr)
{
}

void APMWPlayerController::BeginPlay()
{
	Super::BeginPlay();

	bShowMouseCursor = true;
	SetInputMode(FInputModeGameAndUI());
}

void APMWPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (IsLocalPlayerController())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(IMC_Character, 0);
		}
	}
}
