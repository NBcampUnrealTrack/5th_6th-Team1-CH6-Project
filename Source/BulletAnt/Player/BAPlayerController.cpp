#include "Player/BAPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Building/BuildManagerComponent.h"
#include "Player/BACharacter.h"
#include "UI/UW_PlayerHUDWidget.h"

void ABAPlayerController::BeginPlay()
{
	Super::BeginPlay();
	if (!IsLocalController()) return;
	bIsBuildMode = false;

	if(UEnhancedInputLocalPlayerSubsystem* Subsystem 
		= ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		if (DefaultMappingContext)
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

	if (ABACharacter* PlayerCharacter = Cast<ABACharacter>(GetPawn()))
	{
		if (HUDClass) 
		{
			HUD = CreateWidget<UUW_PlayerHUDWidget>(this, HUDClass);

			HUD->OwnerCharacter = PlayerCharacter;
			HUD->AddToViewport();
		}
	}
}

void ABAPlayerController::SwitchingMode()
{
	bIsBuildMode = !bIsBuildMode;
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem
		= ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		if (bIsBuildMode)
		{
			Subsystem->AddMappingContext(BuildingMappingContext, 1);
			UE_LOG(LogTemp, Log, TEXT("건축 모드 ON"));
		}
		else
		{
			Subsystem->RemoveMappingContext(BuildingMappingContext);
			UE_LOG(LogTemp, Log, TEXT("건축 모드 OFF"));
		}
	}
}