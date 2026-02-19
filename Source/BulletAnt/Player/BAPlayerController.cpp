#include "Player/BAPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Building/BuildManagerComponent.h"
#include "Player/BACharacter.h"
#include "UI/UW_PlayerHUDWidget.h"
#include "UI/UW_RespawnBar.h"
#include "UI/UISubsystem.h"
#include "AbilitySystemComponent.h"
#include "GAS/BAGameplayTags.h"

void ABAPlayerController::BeginPlay()
{
	Super::BeginPlay();
	if (!IsLocalController()) return;

	FInputModeGameOnly GameAndUI;
	SetInputMode(GameAndUI);
	bShowMouseCursor = false;

	bIsBuildMode = false;

	if(UEnhancedInputLocalPlayerSubsystem* Subsystem 
		= ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		if (DefaultMappingContext)
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

	if (PlayerCharacter = Cast<ABACharacter>(GetPawn()))
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

void ABAPlayerController::HandleRespawnBar()
{
	if (!IsValid(this)) return;

	if (!IsValid(RespawnBarUI)) return;

	CurrentTime += 0.1f;

	RespawnBarUI->UpdateRespawnBar(CurrentTime, TotalTime);
}

void ABAPlayerController::StartRespawnBar(float InTotalTime)
{
	if (!PlayerCharacter) return;

	ULocalPlayer* LP = GetLocalPlayer();
	if (!LP) return;

	UISubsystem = LP->GetSubsystem<UUISubsystem>();

	CurrentTime = 0.f;
	TotalTime = InTotalTime;

	if (IsValid(UISubsystem))
	{
		RespawnBarUI = UISubsystem->ShowUI<UUW_RespawnBar>(EUIType::RespawnBar);
	}

	GetWorld()->GetTimerManager().SetTimer(
		RespawnBarTimer,
		this,
		&ABAPlayerController::HandleRespawnBar,
		0.1f,
		true
	);	
}

void ABAPlayerController::StopRespawnBar()
{
	if (!IsValid(this)) return;
	if (!IsValid(RespawnBarUI)) return;
	if (!GetWorld()) return;
	GetWorld()->GetTimerManager().ClearTimer(RespawnBarTimer);
	UISubsystem->HideUI(EUIType::RespawnBar);
}
