#include "UI/UW_GameOver.h"

#include "Player/BAPlayerState.h"
#include "Components/TextBlock.h"
#include "GAS/AttributeSet/EXPAttributeSet.h"
#include "FrameWork/BAGameState.h"
#include "Multiplayer/MultiplayerSubsystem.h"
#include "Framework/BAGameInstance.h"
#include "Framework/MapConfig.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Kismet/GameplayStatics.h"

void UUW_GameOver::NativeConstruct()
{
	Super::NativeConstruct();

	ExitButton->OnClicked.AddDynamic(this, &ThisClass::OnClickedExit);
}

void UUW_GameOver::InitText(bool bIsComplete)
{

	if (ABAPlayerState* PS = Cast<ABAPlayerState>(GetOwningPlayerState()))
	{
		UWorld* World = GetWorld();
		if (KillCountText)
		{
			KillCountText->SetText(FText::AsNumber(PS->GetKillCount()));
		}
		if (DamageText)
		{
			DamageText->SetText(FText::AsNumber(PS->GetTotalDamage()));
		}
		if (LevelText)
		{
			const UEXPAttributeSet* EXPSet = PS->GetAbilitySystemComponent()->GetSet<UEXPAttributeSet>();
			if (EXPSet)
			{
				LevelText->SetText(FText::AsNumber(EXPSet->GetCurrentLevel()));
			}
		}
		if (DaysText)
		{			
			if (IsValid(World))
			{
				ABAGameState* GameState = World->GetGameState<ABAGameState>();
				if (IsValid(GameState))
				{
					DaysText->SetText(FText::AsNumber(GameState->GetDate()));
				}
			}
		}
		if (BuildingText)
		{
			BuildingText->SetText(FText::AsNumber(PS->GetBuildCount()));
		}
		if (WeaponText)
		{
			if (IsValid(World))
			{
				ABAGameState* GS = World->GetGameState<ABAGameState>();
				if (IsValid(GS))
				{
					WeaponText->SetText(FText::AsNumber(GS->GetHaveWeaponArray().Num()));
				}
			}
		}
	}

	SetCompleteImage(bIsComplete);
}

void UUW_GameOver::SetCompleteImage(bool bIsComplete)
{
	if (!FailedImage || !CompleteImage) return;

	if (bIsComplete)
	{
		EndImage->SetBrushFromTexture(CompleteImage);
		EndImage->SetOpacity(1.f);
	}
	else
	{
		EndImage->SetBrushFromTexture(FailedImage);
		EndImage->SetOpacity(1.f);
	}
}

//void UUW_GameOver::GoToLobby()
//{
//	APlayerController* PC = GetOwningPlayer();
//	if (IsValid(PC) == false || PC->IsLocalController() == false)
//		return;
//
//	UBAGameInstance* GameInstance = GetGameInstance<UBAGameInstance>();
//	UMultiplayerSubsystem* MultiplayerSubsystem = IsValid(GameInstance) == true ?  GameInstance->GetSubsystem<UMultiplayerSubsystem>() : nullptr;
//	if (IsValid(MultiplayerSubsystem) == false)
//		return;
//
//	UMapConfig* MapConfig = IsValid(GameInstance) == true ? GameInstance->GetMapConfig() : nullptr;
//	if (IsValid(MapConfig) == false)
//	{
//		//UKismetSystemLibrary::PrintString(GetWorld(), TEXT("Lobby Travel Failed 1"));
//		return;
//	}
//
//	FString LobbyPath = MapConfig->LobbyLevel.ToSoftObjectPath().ToString();
//	FString MapName = FPackageName::ObjectPathToPackageName(LobbyPath);
//	MultiplayerSubsystem->ServerTravelToLevel(MapName);
//}

void UUW_GameOver::OnClickedExit()
{
	UKismetSystemLibrary::QuitGame(this, GetOwningPlayer(), EQuitPreference::Quit, false);
}
