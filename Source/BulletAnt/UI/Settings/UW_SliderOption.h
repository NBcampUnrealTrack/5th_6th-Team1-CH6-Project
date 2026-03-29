// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/Settings/SettingsOptionValue.h"
#include "UW_SliderOption.generated.h"

class UTextBlock;
class USlider;

/**
 * 
 */
UCLASS()
class BULLETANT_API UUW_SliderOption : public UUserWidget, public ISettingsOptionValue
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

public:
	virtual float GetValue() const override;
	virtual void SetValue(float InValue) override;

private:
	UFUNCTION()
	void HandleSliderValueChanged(float InValue);

	void UpdateDisplayText(float InValue) const;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> SliderValue;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USlider> OptionSlider;
};
