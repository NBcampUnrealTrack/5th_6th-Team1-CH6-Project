// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "BABGMManager.generated.h"

UCLASS()
class BULLETANT_API ABABGMManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABABGMManager();
	void StartDayPhase();
	void StartNightPhase();
	void UpdateEnvironmentVolume(float PlayerZ);
protected:
	virtual void BeginPlay() override;
	UPROPERTY(EditAnywhere, Category = "Music")
	TObjectPtr<USoundBase> DayMusic;

	UPROPERTY(EditAnywhere, Category = "Audio|Night")
	TObjectPtr<USoundBase> NightMusic1;
	UPROPERTY(EditAnywhere, Category = "Audio|Night")
	TObjectPtr<USoundBase> NightMusic2;

	UPROPERTY()
	TObjectPtr<UAudioComponent> AudioComp;

	UPROPERTY(EditAnywhere, Category = "Audio|Ambient")
	TObjectPtr<USoundBase> WindSoundAsset;
	UPROPERTY(VisibleAnywhere, Category = "Audio|Ambient")
	UAudioComponent* WindAudioComp;
private:
	UFUNCTION()
	void OnNightMusic1Finished();

	UFUNCTION()
	void StopNightMusicEarly();

	FTimerHandle NightMusicTimerHandle;

	bool bIsPlayingDay;
};
