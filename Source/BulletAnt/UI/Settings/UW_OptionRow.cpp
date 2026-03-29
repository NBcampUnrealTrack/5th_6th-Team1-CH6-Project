// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Settings/UW_OptionRow.h"

#include "Components/TextBlock.h"
#include "Components/NamedSlot.h"
#include "UI/Settings/SettingsOptionValue.h"

void UUW_OptionRow::SetOptionNameText(const FText& InText)
{
	if (OptionName)
	{
		OptionName->SetText(InText);
	}
}

float UUW_OptionRow::GetOptionValue(float DefaultValue) const
{
	if (!RowSlot)
	{
		return DefaultValue;
	}

	UWidget* Content = RowSlot->GetContent();
	if (!Content)
	{
		return DefaultValue;
	}

	if (Content->GetClass()->ImplementsInterface(USettingsOptionValue::StaticClass()))
	{
		if (ISettingsOptionValue* OptionValue = Cast<ISettingsOptionValue>(Content))
		{
			return OptionValue->GetValue();
		}
	}

	return DefaultValue;
}

UWidget* UUW_OptionRow::GetValueWidget() const
{
	return RowSlot ? RowSlot->GetContent() : nullptr;
}