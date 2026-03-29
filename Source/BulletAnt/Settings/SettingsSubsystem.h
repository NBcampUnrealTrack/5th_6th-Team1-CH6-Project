// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SettingsSubsystem.generated.h"

class USoundMix;
class USoundClass;

UENUM(BlueprintType)
enum class EBAWindowMode : uint8
{
	Fullscreen UMETA(DisplayName = "Fullscreen"),
	WindowedFullscreen UMETA(DisplayName = "Borderless Window"),
	Windowed UMETA(DisplayName = "Windowed")
};

UCLASS()
class BULLETANT_API USettingsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	USettingsSubsystem();

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UFUNCTION(BlueprintCallable, Category = "Settings|Video")
	void SetResolution(const FIntPoint& Resolution, EBAWindowMode WindowMode, bool bApply);

	UFUNCTION(BlueprintCallable, Category = "Settings|Video")
	void ApplyVideoSettings(bool bCheckForCommandLineOverrides);

	UFUNCTION(BlueprintCallable, Category = "Settings|Video")
	TArray<FIntPoint> GetSupportedResolutions() const;

	UFUNCTION(BlueprintCallable, Category = "Settings|Video")
	FIntPoint GetCurrentResolution() const;

	UFUNCTION(BlueprintCallable, Category = "Settings|Video")
	EBAWindowMode GetCurrentWindowMode() const;

	UFUNCTION(BlueprintCallable, Category = "Settings|Audio")
	void SetMasterVolume(float Volume);

	UFUNCTION(BlueprintCallable, Category = "Settings|Audio")
	void SetMusicVolume(float Volume);

	UFUNCTION(BlueprintCallable, Category = "Settings|Audio")
	void SetSfxVolume(float Volume);

	UFUNCTION(BlueprintCallable, Category = "Settings|Audio")
	void ConfigureSoundMix(USoundMix* InSoundMix, USoundClass* InMasterClass, USoundClass* InMusicClass, USoundClass* InSfxClass);

	UFUNCTION(BlueprintCallable, Category = "Settings|Audio")
	float GetMasterVolume() const;

	UFUNCTION(BlueprintCallable, Category = "Settings|Audio")
	float GetMusicVolume() const;

	UFUNCTION(BlueprintCallable, Category = "Settings|Audio")
	float GetSfxVolume() const;

private:
	void ApplySoundMix();

	UPROPERTY(EditDefaultsOnly, Category = "Settings|Audio")
	TObjectPtr<USoundMix> MasterSoundMix;

	UPROPERTY(EditDefaultsOnly, Category = "Settings|Audio")
	TObjectPtr<USoundClass> MasterSoundClass;

	UPROPERTY(EditDefaultsOnly, Category = "Settings|Audio")
	TObjectPtr<USoundClass> MusicSoundClass;

	UPROPERTY(EditDefaultsOnly, Category = "Settings|Audio")
	TObjectPtr<USoundClass> SfxSoundClass;

	float MasterVolume;
	float MusicVolume;
	float SfxVolume;
};
