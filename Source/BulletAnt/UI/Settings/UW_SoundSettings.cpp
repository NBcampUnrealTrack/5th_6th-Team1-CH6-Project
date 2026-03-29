// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Settings/UW_SoundSettings.h"

#include "Settings/SettingsSubsystem.h"
#include "UI/Settings/UW_OptionRow.h"
#include "UI/Settings/UW_SliderOption.h"
#include "Components/NamedSlot.h"

void UUW_SoundSettings::NativeConstruct()
{
	Super::NativeConstruct();

	SyncFromSubsystem();
}

void UUW_SoundSettings::SyncFromSubsystem()
{
	MasterVolume->SetOptionNameText(FText::FromString("Master Volume"));
	MusicVolume->SetOptionNameText(FText::FromString("Music Volume"));
	SfxVolume->SetOptionNameText(FText::FromString("Sfx Volume"));

	USettingsSubsystem* Subsys = GetGameInstance() ? GetGameInstance()->GetSubsystem<USettingsSubsystem>() : nullptr;
	if (!Subsys)
	{
		return;
	}

	auto SyncRow = [](UUW_OptionRow* Row, float Value)
		{
			if (!Row)
			{
				return;
			}

			UWidget* W = Row->GetValueWidget();
			if (!W)
			{
				return;
			}

			if (W->GetClass()->ImplementsInterface(USettingsOptionValue::StaticClass()))
			{
				if (ISettingsOptionValue* Opt = Cast<ISettingsOptionValue>(W))
				{
					Opt->SetValue(Value);
				}
			}
		};

	SyncRow(MasterVolume, Subsys->GetMasterVolume());
	SyncRow(MusicVolume, Subsys->GetMusicVolume());
	SyncRow(SfxVolume, Subsys->GetSfxVolume());
}

void UUW_SoundSettings::ApplyToSubsystem()
{
	USettingsSubsystem* Subsys = GetGameInstance() ? GetGameInstance()->GetSubsystem<USettingsSubsystem>() : nullptr;
	if (!Subsys)
	{
		return;
	}

	const float Master = MasterVolume ? MasterVolume->GetOptionValue(1.f) : 1.f;
	const float Music = MusicVolume ? MusicVolume->GetOptionValue(1.f) : 1.f;
	const float Sfx = SfxVolume ? SfxVolume->GetOptionValue(1.f) : 1.f;

	Subsys->SetMasterVolume(Master);
	Subsys->SetMusicVolume(Music);
	Subsys->SetSfxVolume(Sfx);
}
