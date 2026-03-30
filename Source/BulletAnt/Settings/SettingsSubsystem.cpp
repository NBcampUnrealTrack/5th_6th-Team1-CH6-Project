// Fill out your copyright notice in the Description page of Project Settings.

#include "Settings/SettingsSubsystem.h"

#include "Engine/Engine.h"
#include "GameFramework/GameUserSettings.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Sound/SoundClass.h"
#include "Sound/SoundMix.h"

USettingsSubsystem::USettingsSubsystem()
	: MasterSoundMix(nullptr)
	, MasterSoundClass(nullptr)
	, MusicSoundClass(nullptr)
	, SfxSoundClass(nullptr)
	, MasterVolume(1.0f)
	, MusicVolume(1.0f)
	, SfxVolume(1.0f)
{
}

void USettingsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	ApplySoundMix();
}

static EWindowMode::Type ToWindowMode(EBAWindowMode InMode)
{
	switch (InMode)
	{
	case EBAWindowMode::Fullscreen:
		return EWindowMode::Fullscreen;
	case EBAWindowMode::WindowedFullscreen:
		return EWindowMode::WindowedFullscreen;
	case EBAWindowMode::Windowed:
	default:
		return EWindowMode::Windowed;
	}
}

static EBAWindowMode FromWindowMode(EWindowMode::Type InMode)
{
	switch (InMode)
	{
	case EWindowMode::Fullscreen:
		return EBAWindowMode::Fullscreen;
	case EWindowMode::WindowedFullscreen:
		return EBAWindowMode::WindowedFullscreen;
	case EWindowMode::Windowed:
	default:
		return EBAWindowMode::Windowed;
	}
}

void USettingsSubsystem::SetResolution(const FIntPoint& Resolution, EBAWindowMode WindowMode, bool bApply)
{
	if (UGameUserSettings* Settings = GEngine ? GEngine->GetGameUserSettings() : nullptr)
	{
		Settings->SetScreenResolution(Resolution);
		Settings->SetFullscreenMode(ToWindowMode(WindowMode));

		if (bApply)
		{
			Settings->ApplySettings(false);
		}
	}
}

void USettingsSubsystem::ApplyVideoSettings(bool bCheckForCommandLineOverrides)
{
	if (UGameUserSettings* Settings = GEngine ? GEngine->GetGameUserSettings() : nullptr)
	{
		Settings->ApplySettings(bCheckForCommandLineOverrides);
	}
}

TArray<FIntPoint> USettingsSubsystem::GetSupportedResolutions() const
{
	TArray<FIntPoint> Resolutions;

	if (UGameUserSettings* Settings = GEngine ? GEngine->GetGameUserSettings() : nullptr)
	{
		UKismetSystemLibrary::GetSupportedFullscreenResolutions(Resolutions);
	}

	if (Resolutions.Num() == 0)
	{
		Resolutions.Add(GetCurrentResolution());
	}

	return Resolutions;
}

FIntPoint USettingsSubsystem::GetCurrentResolution() const
{
	if (UGameUserSettings* Settings = GEngine ? GEngine->GetGameUserSettings() : nullptr)
	{
		return Settings->GetScreenResolution();
	}

	return FIntPoint::ZeroValue;
}

EBAWindowMode USettingsSubsystem::GetCurrentWindowMode() const
{
	if (UGameUserSettings* Settings = GEngine ? GEngine->GetGameUserSettings() : nullptr)
	{
		return FromWindowMode(Settings->GetFullscreenMode());
	}

	return EBAWindowMode::Windowed;
}

void USettingsSubsystem::SetMasterVolume(float Volume)
{
	MasterVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
	ApplySoundMix();
}

void USettingsSubsystem::SetMusicVolume(float Volume)
{
	MusicVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
	ApplySoundMix();
}

void USettingsSubsystem::SetSfxVolume(float Volume)
{
	SfxVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
	ApplySoundMix();
}

void USettingsSubsystem::ConfigureSoundMix(USoundMix* InSoundMix, USoundClass* InMasterClass, USoundClass* InMusicClass, USoundClass* InSfxClass)
{
	MasterSoundMix = InSoundMix;
	MasterSoundClass = InMasterClass;
	MusicSoundClass = InMusicClass;
	SfxSoundClass = InSfxClass;
	ApplySoundMix();
}

float USettingsSubsystem::GetMasterVolume() const
{
	return MasterVolume;
}

float USettingsSubsystem::GetMusicVolume() const
{
	return MusicVolume;
}

float USettingsSubsystem::GetSfxVolume() const
{
	return SfxVolume;
}

void USettingsSubsystem::ApplySoundMix()
{
	if (!MasterSoundMix)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	UGameplayStatics::PushSoundMixModifier(World, MasterSoundMix);

	if (MasterSoundClass)
	{
		UGameplayStatics::SetSoundMixClassOverride(
			World,
			MasterSoundMix,
			MasterSoundClass,
			MasterVolume,
			1.0f,
			0.0f,
			true
		);
	}

	if (MusicSoundClass)
	{
		UGameplayStatics::SetSoundMixClassOverride(
			World,
			MasterSoundMix,
			MusicSoundClass,
			MasterVolume * MusicVolume,
			1.0f,
			0.0f,
			true
		);
	}

	if (SfxSoundClass)
	{
		UGameplayStatics::SetSoundMixClassOverride(
			World,
			MasterSoundMix,
			SfxSoundClass,
			MasterVolume * SfxVolume,
			1.0f,
			0.0f,
			true
		);
	}
}
