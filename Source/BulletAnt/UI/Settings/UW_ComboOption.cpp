// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Settings/UW_ComboOption.h"

#include "Components/ComboBoxString.h"

float UUW_ComboOption::GetValue() const
{
	return static_cast<float>(OptionCombo->GetSelectedIndex());
}

void UUW_ComboOption::SetValue(float InValue)
{
	if (!OptionCombo || CachedOptions.Num() == 0)
	{
		return;
	}

	const int32 Index = FMath::Clamp(FMath::RoundToInt(InValue), 0, CachedOptions.Num() - 1);
	OptionCombo->SetSelectedOption(CachedOptions[Index]);
}

void UUW_ComboOption::SetOptions(const TArray<FString>& InOptions)
{
	CachedOptions = InOptions;

	if (!OptionCombo)
	{
		return;
	}

	OptionCombo->ClearOptions();
	for (const FString& Opt : InOptions)
	{
		OptionCombo->AddOption(Opt);
	}

	if (InOptions.Num() > 0)
	{
		OptionCombo->SetSelectedOption(InOptions[0]);
	}
}
