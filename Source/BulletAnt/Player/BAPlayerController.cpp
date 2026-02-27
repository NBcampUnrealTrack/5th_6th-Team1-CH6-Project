#include "Player/BAPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Building/BuildManagerComponent.h"
#include "Player/BACharacter.h"
#include "UI/UW_PlayerHUDWidget.h"
#include "UI/UW_RespawnBar.h"
#include "UI/UISubsystem.h"
#include "AbilitySystemComponent.h"
#include "GAS/BAGameplayTags.h"
#include "UI/UW_OreCount.h"
#include "Framework/BAGameState.h"


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

	if (ABACharacter* PlayerCharacter = Cast<ABACharacter>(GetPawn()))
	{
		if (HUDClass) 
		{
			HUD = CreateWidget<UUW_PlayerHUDWidget>(this, HUDClass);

			HUD->OwnerCharacter = PlayerCharacter;
			HUD->AddToViewport();
		}
	}

	ULocalPlayer* LP = GetLocalPlayer();
	if (!LP) return;

	UISubsystem = LP->GetSubsystem<UUISubsystem>();
	if (IsValid(UISubsystem) == true)
	{
		UUW_OreCount* OreCountUI = UISubsystem->ShowUI<UUW_OreCount>(EUIType::OreCount);
		if (IsValid(OreCountUI) == true)
		{
			ABAGameState* GS = GetWorld()->GetGameState<ABAGameState>();
			if (IsValid(GS) == true)
			{
				FOnOreChanged::FDelegate Delegate;
				Delegate.BindDynamic(OreCountUI, &UUW_OreCount::SetOreCount);
				GS->BindOnOreChanged(Delegate);
			}
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



void ABAPlayerController::SwitchGroundScanner()
{
	bActiveGroundScannerUI ^= 1;

	if (bActiveGroundScannerUI == true)
	{
		FInputModeGameAndUI InputUIMode;
		SetInputMode(InputUIMode);
		bShowMouseCursor = true;
		if (auto* LP = GetLocalPlayer())
		{
			if (auto* UIS = LP->GetSubsystem<UUISubsystem>())
			{
				UIS->ShowUI<UUserWidget>(EUIType::GroundScanner);
			}
		}
	}
	else
	{
		FInputModeGameOnly InputGameMode;
		SetInputMode(InputGameMode);
		bShowMouseCursor = false;
		if (auto* LP = GetLocalPlayer())
		{
			if (auto* UIS = LP->GetSubsystem<UUISubsystem>())
			{
				UIS->HideUI(EUIType::GroundScanner);
			}
		}
	}
}
