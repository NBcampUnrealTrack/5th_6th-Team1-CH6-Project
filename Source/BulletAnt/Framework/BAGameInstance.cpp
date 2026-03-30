#include "Framework/BAGameInstance.h"
#include "Framework/MapConfig.h"
#include "Settings/SettingsSubsystem.h"
#include "Sound/SoundClass.h"
#include "Sound/SoundMix.h"

void UBAGameInstance::Init()
{
	Super::Init();

	USettingsSubsystem* SettingsSubsystem = GetSubsystem<USettingsSubsystem>();
	if (!SettingsSubsystem)
	{
		return;
	}

	SettingsSubsystem->ConfigureSoundMix(
		MasterSoundMix,
		MasterSoundClass,
		MusicSoundClass,
		SfxSoundClass
	);
}

void UBAGameInstance::OnStart()
{
	Super::OnStart();
}
