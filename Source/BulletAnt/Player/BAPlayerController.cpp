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
			HUD->AddToViewport(0);
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

				const auto& OreInventory = GS->GetOreInventory();
				for (const auto& OrePair : OreInventory)
				{
					OreCountUI->SetOreCount(OrePair.Key, OrePair.Value);
				}
			}
		}

		USpawnManagerSubsystem* SpawnManager = GetWorld()->GetSubsystem<USpawnManagerSubsystem>();
		if (IsValid(SpawnManager))
		{
			WaveTimerUI = UISubsystem->ShowUI<UUW_WaveTimer>(EUIType::WaveTimer);
		}
	}
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



void ABAPlayerController::StartADSUI()
{

}

void ABAPlayerController::StopADSUI()
{

}

void ABAPlayerController::Server_RequestBuyGacha_Implementation(ABaseShop* InShop, int32 GachaID,int32 Count)
{
	if (!InShop) return;

	InShop->BuyGacha(GachaID, Count);
}

void ABAPlayerController::Server_RequestAddWeapon_Implementation(TSubclassOf<ABaseWeapon> InWeaponClass)
{
	if (!HasAuthority()) return;
	
	ABAGameState* GS = Cast<ABAGameState>(GetWorld()->GetGameState());
	if (!GS) return;

	GS->AddHaveWeapon(InWeaponClass);
}

void ABAPlayerController::Server_RequestDeleteBox_Implementation(ABAItemBox* InItemBox)
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

void ABAPlayerController::Server_RequestWeaponLog_Implementation(UWeaponDataAsset* InData)
{
	Multicast_ShowWeaponLog(InData);
}

void ABAPlayerController::Multicast_ShowWeaponLog_Implementation(UWeaponDataAsset* InData)
{
	if (HUD)
	{
		HUD->AddWeaponLog(InData);
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
