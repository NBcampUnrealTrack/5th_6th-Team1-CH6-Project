// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Settings/UW_VideoSettings.h"

#include "UI/Settings/UW_OptionRow.h"
#include "UI/Settings/UW_ComboOption.h"
#include "UI/Settings/SettingsOptionValue.h"
#include "Settings/SettingsSubsystem.h"

void UUW_VideoSettings::NativeConstruct()
{
	Super::NativeConstruct();

	BuildOptions();
	SyncFromSubsystem();
}

void UUW_VideoSettings::SyncFromSubsystem()
{
	USettingsSubsystem* Subsys = GetGameInstance() ? GetGameInstance()->GetSubsystem<USettingsSubsystem>() : nullptr;
	if (!Subsys)
	{
		return;
	}

	const EBAWindowMode CurrentMode = Subsys->GetCurrentWindowMode();
	int32 ModeFound = WindowModeMap.IndexOfByKey(CurrentMode);
	if (ModeFound == INDEX_NONE)
	{
		ModeFound = 0;
	}

	if (UUW_ComboOption* Combo = GetComboFromRow(WindowMode))
	{
		Combo->SetValue(static_cast<float>(ModeFound));
	}

	const FIntPoint CurrentResolution = Subsys->GetCurrentResolution();
	int32 ResolutionFound = ResolutionMap.IndexOfByKey(CurrentResolution);
	if (ResolutionFound == INDEX_NONE)
	{
		ResolutionFound = 0;
	}

	if (UUW_ComboOption* Combo = GetComboFromRow(Resolution))
	{
		Combo->SetValue(static_cast<float>(ResolutionFound));
	}

}

void UUW_VideoSettings::ApplyToSubsystem()
{
	USettingsSubsystem* Subsys = GetGameInstance() ? GetGameInstance()->GetSubsystem<USettingsSubsystem>() : nullptr;
	if (!Subsys)
	{
		return;
	}

	const int32 WindowModeIndex = WindowMode ? FMath::RoundToInt(WindowMode->GetOptionValue(0.f)) : 0;
	const int32 ResolutionIndex = Resolution ? FMath::RoundToInt(Resolution->GetOptionValue(0.f)) : 0;

	const int32 WM = FMath::Clamp(WindowModeIndex, 0, WindowModeMap.Num() - 1);
	const int32 RS = FMath::Clamp(ResolutionIndex, 0, ResolutionMap.Num() - 1);

	const EBAWindowMode Mode = WindowModeMap.IsValidIndex(WM) ? WindowModeMap[WM] : EBAWindowMode::Windowed;
	const FIntPoint Res = ResolutionMap.IsValidIndex(RS) ? ResolutionMap[RS] : FIntPoint(1280, 720);

	Subsys->SetResolution(Res, Mode, false);
	Subsys->ApplyVideoSettings(false);
}

void UUW_VideoSettings::BuildOptions()
{
	WindowMode->SetOptionNameText(FText::FromString("Window Mode"));
	Resolution->SetOptionNameText(FText::FromString("Resolution"));

	USettingsSubsystem* Subsys = GetGameInstance() ? GetGameInstance()->GetSubsystem<USettingsSubsystem>() : nullptr;
	if (!Subsys)
	{
		return;
	}

	WindowModeMap = { EBAWindowMode::Fullscreen, EBAWindowMode::WindowedFullscreen, EBAWindowMode::Windowed };
	TArray<FString> WindowModeLabels = { TEXT("Fullscreen"), TEXT("Borderless Window"), TEXT("Windowed") };

	if (UUW_ComboOption* Combo = GetComboFromRow(WindowMode))
	{
		Combo->SetOptions(WindowModeLabels);
	}

	ResolutionMap = Subsys->GetSupportedResolutions();
	TArray<FString> ResLabels;
	ResLabels.Reserve(ResolutionMap.Num());

	for (const FIntPoint& R : ResolutionMap)
	{
		ResLabels.Add(FString::Printf(TEXT("%d x %d"), R.X, R.Y));
	}

	if (UUW_ComboOption* Combo = GetComboFromRow(Resolution))
	{
		Combo->SetOptions(ResLabels);
	}
}

UUW_ComboOption* UUW_VideoSettings::GetComboFromRow(UUW_OptionRow* Row)
{
	if (!Row)
	{
		return nullptr;
	}

	return Cast<UUW_ComboOption>(Row->GetValueWidget());
}
