// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/Settings/SettingsOptionValue.h"
#include "UW_ComboOption.generated.h"

class UComboBoxString;

/**
 * 
 */
UCLASS()
class BULLETANT_API UUW_ComboOption : public UUserWidget, public ISettingsOptionValue
{
	GENERATED_BODY()

public:
	virtual float GetValue() const override;
	virtual void SetValue(float InValue) override;

	void SetOptions(const TArray<FString>& InOptions);

private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UComboBoxString> OptionCombo;

	TArray<FString> CachedOptions;
};
