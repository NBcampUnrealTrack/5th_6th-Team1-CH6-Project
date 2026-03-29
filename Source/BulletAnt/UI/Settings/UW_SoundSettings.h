// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UW_SoundSettings.generated.h"

class UUW_OptionRow;

/**
 * 
 */
UCLASS()
class BULLETANT_API UUW_SoundSettings : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;

public:
	void SyncFromSubsystem();
	void ApplyToSubsystem();

private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UUW_OptionRow> MasterVolume;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UUW_OptionRow> MusicVolume;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UUW_OptionRow> SfxVolume;
};
