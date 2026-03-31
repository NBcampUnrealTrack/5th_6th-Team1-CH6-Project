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
#include "UI/UW_WaveTimer.h"
#include "Enemy/Spawn/SpawnManagerSubsystem.h"
#include "Building/BaseShop.h"
#include "UI/UW_ShopWindow.h"
#include "Shop/BAItemBox.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "UI/UW_WeaponLog.h"
#include "UI/UW_Compass.h"
#include "Player/BAPlayerState.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Weapon/BaseWeapon.h"
#include "Weapon/BaseRangedWeapon.h"
#include "GAS/AttributeSet/AmmoAttributeSet.h"
#include "Framework/BAGameMode.h"
#include "UI/UW_Loading.h"
#include "Audio/BABGMManager.h"
#include "Kismet/GameplayStatics.h"
#include "Multiplayer/MultiplayerSubsystem.h"

void ABAPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalController() == false)
		return;

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem
		= ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		if (DefaultMappingContext)
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
	if (PlayerCameraManager)
	{
		PlayerCameraManager->ViewPitchMin = -60.0f; // 아래 제한
		PlayerCameraManager->ViewPitchMax = 85.0f;  // 위 제한
	}
	SetupForLobby();
}

void ABAPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (IsValid(WaveTimerUI))
	{
		WaveTimerUI->RemoveFromParent();
		WaveTimerUI = nullptr;
	}

	Super::EndPlay(EndPlayReason);
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
	if (CurrentTime > TotalTime)
	{
		StopRespawnBar();
	}

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

void ABAPlayerController::Server_RequestBuyGacha_Implementation(ABaseShop* InShop, int32 GachaID,int32 Count)
{
	if (!InShop) return;

	InShop->BuyGacha(GachaID, Count);
}

void ABAPlayerController::Server_RequestAddWeapon_Implementation(ABAItemBox* InItemBox)
{
	if (!HasAuthority()) return;
	
	ABAGameState* GS = Cast<ABAGameState>(GetWorld()->GetGameState());
	if (!GS) return;

	if (!InItemBox->GetItem())
	{
		InItemBox->AddWeaponToItemBox();
	}

	if (InItemBox->GetItem())
	{
		GS->AddHaveWeapon(InItemBox->GetItem());
	}

	RequestDeleteBox(InItemBox);

	ABaseWeapon* WeaponCDO = Cast<ABaseWeapon>(InItemBox->GetItem()->GetDefaultObject());
	if (WeaponCDO)
	{
		ABACharacter* PlayerCharacter = Cast<ABACharacter>(GetPawn());
		PlayerCharacter->RequestWeaponLog(WeaponCDO->GetWeaponData());
	}
}

void ABAPlayerController::RequestDeleteBox(ABAItemBox* InItemBox)
{
	if (!InItemBox) return;
	if (InItemBox->GetbIsUsed()) return;

	InItemBox->SetbIsUsed(true);

	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetPawn());
	if (ASC)
	{
		FGameplayCueParameters Params;
		Params.SourceObject = InItemBox;
		ASC->ExecuteGameplayCue(TAG_GameplayCue_Shop_UseItemBox, Params);
	}
}

void ABAPlayerController::ShowShopUI()
{
	if (IsValid(UISubsystem))
	{
		RespawnBarUI = UISubsystem->ShowUI<UUW_RespawnBar>(EUIType::RespawnBar);
	}
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

void ABAPlayerController::SetLevelType(ELevelType InType)
{
	LevelType = InType;
	Client_SetupController(LevelType);
}

void ABAPlayerController::Client_SetVoiceChatUser_Implementation()
{
	UMultiplayerSubsystem* MultiplayerSubsystem = GetGameInstance()->GetSubsystem<UMultiplayerSubsystem>();
	if (IsValid(MultiplayerSubsystem) == true)
	{
		MultiplayerSubsystem->SetVoiceChatUser();
		MultiplayerSubsystem->RefreshOtherVoices();
	}
}

void ABAPlayerController::Client_SetupController_Implementation(ELevelType InType)
{
	LevelType = InType;
	SetupController();
}

void ABAPlayerController::SetupController()
{
	if (IsLocalController() == false)
		return;

	UWorld* World = GetWorld();
	ABAGameState* GS = IsValid(World) == true ? World->GetGameState<ABAGameState>() : nullptr;
	APawn* MyPawn = GetPawn();
	if (IsValid(GS) == false || IsValid(MyPawn) == false)
	{
		World->GetTimerManager().SetTimerForNextTick(this, &ThisClass::SetupController);
		return;
	}

	switch (LevelType)
	{
		case ELevelType::Lobby:
			SetupForLobby();
			break;
		case ELevelType::Main:
			SetupForMain();
			break;
		default:
			break;
	}
}

void ABAPlayerController::SetupForLobby()
{
	FInputModeGameOnly GameAndUI;
	SetInputMode(GameAndUI);
	bShowMouseCursor = false;

	bIsBuildMode = false;

	ULocalPlayer* LP = GetLocalPlayer();
	if (!LP) return;

	UISubsystem = LP->GetSubsystem<UUISubsystem>();
	if (IsValid(UISubsystem) == true)
	{
		UISubsystem->ResetAllUI();
		UISubsystem->InitRootHUD();

	}
}

void ABAPlayerController::SetupForMain()
{
	ABAPlayerState* BAPS = GetPlayerState<ABAPlayerState>();
	if (IsValid(BAPS) == true)
	{
		BAPS->SetReadyToStart(false);
	}

	FInputModeUIOnly InputUI;
	SetInputMode(InputUI);
	bShowMouseCursor = false;

	bIsBuildMode = false;

	ULocalPlayer* LP = GetLocalPlayer();
	if (!LP) return;

	UISubsystem = LP->GetSubsystem<UUISubsystem>();
	if (IsValid(UISubsystem) == true)
	{
		UISubsystem->ResetAllUI();
		UISubsystem->InitRootHUD();

		HUD = UISubsystem->ShowUI<UUW_PlayerHUDWidget>(EUIType::PlayerHUD);
		if (HUD)
		{
			ABACharacter* PlayerCharacter = Cast<ABACharacter>(GetPawn());
			if (PlayerCharacter)
			{
				HUD->OwnerCharacter = PlayerCharacter;
				ABAPlayerState* PS = Cast<ABAPlayerState>(PlayerCharacter->GetPlayerState());
				if (PS)
				{
					const UAmmoAttributeSet* Ammo = PS->GetAmmoAttributeSet();
					HUD->UpdateAmmo(Ammo->GetCurrentAmmo(), Ammo->GetMaxAmmo());
					HUD->HideAmmoText();
				}		
			}
			
			HUD->InitPlayerHUD();
		}

		UUW_Compass* CompassUI = UISubsystem->ShowUI<UUW_Compass>(EUIType::Compass);

		USpawnManagerSubsystem* SpawnManager = GetWorld()->GetSubsystem<USpawnManagerSubsystem>();
		if (IsValid(SpawnManager))
		{
			WaveTimerUI = UISubsystem->ShowUI<UUW_WaveTimer>(EUIType::WaveTimer);
		}

		UUW_Loading* LoadingUI = UISubsystem->ShowUI<UUW_Loading>(EUIType::Loading);
		if (IsValid(LoadingUI) == true)
		{
			LoadingUI->ShowLoadingPanel(true);
		}
	}
}

void ABAPlayerController::Client_StartGame_Implementation()
{
	FInputModeGameOnly GameAndUI;
	SetInputMode(GameAndUI);

	ULocalPlayer* LP = GetLocalPlayer();
	if (!LP) return;

	UISubsystem = LP->GetSubsystem<UUISubsystem>();
	if (IsValid(UISubsystem) == true)
	{
		UUW_Loading* LoadingUI = UISubsystem->ShowUI<UUW_Loading>(EUIType::Loading);
		if (IsValid(LoadingUI) == true)
		{
			LoadingUI->ShowLoadingPanel(false);
		}
	}
}

void ABAPlayerController::Server_ReadyToStart_Implementation()
{
	ABAPlayerState* BAPS = GetPlayerState<ABAPlayerState>();
	if (IsValid(BAPS) == false)
		return;

	if (BAPS->GetReadyToStart() == true)
		return;

	BAPS->SetReadyToStart(true);

	ABAGameMode* GM = GetWorld()->GetAuthGameMode<ABAGameMode>();
	if (IsValid(GM) == true)
	{
		GM->CheckAllPlayersReadyToStart();
		if (GM->GetGameStarted() == true)
		{
			Client_StartGame();
		}
	}
}

void ABAPlayerController::ShowAmmo()
{
	if (IsLocalController())
	{
		if (UISubsystem)
		{
			if (HUD)
			{
				HUD->ShowAmmoText();
			}
		}
	}
}

void ABAPlayerController::HideAmmo()
{
	if (IsLocalController())
	{
		if (UISubsystem)
		{
			if (HUD)
			{
				HUD->HideAmmoText();
			}
		}
	}
}


