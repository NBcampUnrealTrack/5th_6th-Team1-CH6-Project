// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Settings/UW_SliderOption.h"

#include "Components/Slider.h"
#include "Components/TextBlock.h"

void UUW_SliderOption::NativeConstruct()
{
	Super::NativeConstruct();

	if (OptionSlider)
	{
		OptionSlider->OnValueChanged.AddDynamic(this, &UUW_SliderOption::HandleSliderValueChanged);
		UpdateDisplayText(OptionSlider->GetValue());
	}
}

float UUW_SliderOption::GetValue() const
{
	return OptionSlider ? OptionSlider->GetValue() : 0.f;
}

void UUW_SliderOption::SetValue(float InValue)
{
	if (!OptionSlider)
	{
		return;
	}

	const float Clamped = FMath::Clamp(InValue, 0.f, 1.f);
	OptionSlider->SetValue(Clamped);
	UpdateDisplayText(Clamped);
}

void UUW_SliderOption::HandleSliderValueChanged(float InValue)
{
	UpdateDisplayText(InValue);
}

void UUW_SliderOption::UpdateDisplayText(float InValue) const
{
	if (!SliderValue)
	{
		return;
	}

	const int32 Percent = FMath::RoundToInt(FMath::Clamp(InValue, 0.f, 1.f) * 100.f);
	SliderValue->SetText(FText::FromString(FString::Printf(TEXT("%d%%"), Percent)));
}