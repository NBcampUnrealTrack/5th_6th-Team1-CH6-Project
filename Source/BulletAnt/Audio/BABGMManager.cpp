// Fill out your copyright notice in the Description page of Project Settings.


#include "Audio/BABGMManager.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ABABGMManager::ABABGMManager()
{
	PrimaryActorTick.bCanEverTick = false;

	AudioComp = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComponent"));
	AudioComp->bAutoActivate = false;
	RootComponent = AudioComp;
	bIsPlayingDay = false;
	WindAudioComp = CreateDefaultSubobject<UAudioComponent>(TEXT("WindAudioComponent"));
	WindAudioComp->SetupAttachment(RootComponent);
	WindAudioComp->bAutoActivate = false;
}
void ABABGMManager::BeginPlay()
{
	Super::BeginPlay();
	if (WindSoundAsset)
	{
		WindAudioComp->SetSound(WindSoundAsset);
		WindAudioComp->Play();
	}
}
void ABABGMManager::StartDayPhase()
{
	if (!DayMusic || bIsPlayingDay) return;
	bIsPlayingDay = true;
	GetWorldTimerManager().ClearTimer(NightMusicTimerHandle);
	AudioComp->OnAudioFinished.Clear();

	AudioComp->SetSound(DayMusic);

	AudioComp->SetUISound(false);
	AudioComp->Play();
}

void ABABGMManager::StartNightPhase()
{
	if (!NightMusic1 || !NightMusic2 || !bIsPlayingDay) return;
	bIsPlayingDay = false;

	AudioComp->FadeOut(1.0f, 0.0f);
	AudioComp->SetSound(NightMusic1);
	AudioComp->Play();

	float SwitchTime = NightMusic1->GetDuration() - 6.0f;

	if (SwitchTime > 0.0f)
	{
		GetWorldTimerManager().SetTimer(
			NightMusicTimerHandle,
			this,
			&ABABGMManager::OnNightMusic1Finished,
			SwitchTime,
			false
		);
	}
	else
	{
		OnNightMusic1Finished();
	}

}

void ABABGMManager::OnNightMusic1Finished()
{
	if (!AudioComp || !NightMusic2) return;

	AudioComp->FadeOut(1.0f, 0.0f);
	AudioComp->SetSound(NightMusic2);
	AudioComp->FadeIn(1.0f, 1.0f);

}

void ABABGMManager::StopNightMusicEarly()
{
	if (AudioComp->IsPlaying())
	{
		AudioComp->FadeOut(1.0f, 0.0f);
		UE_LOG(LogTemp, Warning, TEXT("밤 음악 2를 3초 일찍 종료합니다."));
	}
}

void ABABGMManager::UpdateEnvironmentVolume(float PlayerZ)
{
	float TargetVolume = (PlayerZ < 0.0f) ? 0.0f : 1.0f;

	float CurrentVolume = WindAudioComp->VolumeMultiplier;
	float NewVolume = FMath::FInterpTo(CurrentVolume, TargetVolume, 0.1f, 2.0f);

	WindAudioComp->SetVolumeMultiplier(NewVolume);
}