// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Settings/UW_OptionBox.h"

#include "Components/Button.h"
#include "Components/WidgetSwitcher.h"
#include "UI/Settings/UW_VideoSettings.h"
#include "UI/Settings/UW_SoundSettings.h"
#include "UI/UISubsystem.h"

void UUW_OptionBox::NativeConstruct()
{
	Super::NativeConstruct();

	if (VideoBtn) 
	{ 
		VideoBtn->OnClicked.AddDynamic(this, &UUW_OptionBox::HandleVideoTab);
	}

	if (AudioBtn) 
	{
		AudioBtn->OnClicked.AddDynamic(this, &UUW_OptionBox::HandleAudioTab);
	}

	if (ApplyBtn) 
	{
		ApplyBtn->OnClicked.AddDynamic(this, &UUW_OptionBox::HandleApply); 
	}

	if (ExitBtn) 
	{
		ExitBtn->OnClicked.AddDynamic(this, &UUW_OptionBox::HandleExit); 
	}
}

void UUW_OptionBox::HandleVideoTab()
{
	if (SettingSwitcher)
	{
		SettingSwitcher->SetActiveWidgetIndex(0);
	}
}

void UUW_OptionBox::HandleAudioTab()
{
	if (SettingSwitcher)
	{
		SettingSwitcher->SetActiveWidgetIndex(1);
	}
}

void UUW_OptionBox::HandleApply()
{
	if (!SettingSwitcher)
	{
		return;
	}

	UWidget* Active = SettingSwitcher->GetActiveWidget();
	if (UUW_VideoSettings* Video = Cast<UUW_VideoSettings>(Active))
	{
		Video->ApplyToSubsystem();
	}
	else if (UUW_SoundSettings* Sound = Cast<UUW_SoundSettings>(Active))
	{
		Sound->ApplyToSubsystem();
	}
}

void UUW_OptionBox::HandleExit()
{
	if (ULocalPlayer* LP = GetOwningLocalPlayer())
	{
		if (UUISubsystem* UISubsys = LP->GetSubsystem<UUISubsystem>())
		{
			UISubsys->HideUI(EUIType::Option);
			return;
		}
	}
}