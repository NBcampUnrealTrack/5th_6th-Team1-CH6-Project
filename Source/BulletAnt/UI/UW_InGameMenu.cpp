#include "UI/UW_InGameMenu.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "UI/UISubsystem.h"
#include "GameFramework/PlayerController.h"

void UUW_InGameMenu::NativeConstruct()
{
	Super::NativeConstruct();

	if (ContinueButton)
	{
		ContinueButton->OnClicked.AddDynamic(this, &UUW_InGameMenu::OnClickedContinue);
	}

	if (OptionButton)
	{
		OptionButton->OnClicked.AddDynamic(this, &UUW_InGameMenu::OnClickedOption);
	}

	if (ExitButton)
	{
		ExitButton->OnClicked.AddDynamic(this, &UUW_InGameMenu::OnClickedExit);
	}
}

void UUW_InGameMenu::OnClickedContinue()
{
	APlayerController* PC = GetOwningPlayer();
	if (!PC)
	{
		return;
	}

	ULocalPlayer* LP = PC->GetLocalPlayer();
	if (!LP)
	{
		return;
	}

	UUISubsystem* UIS = LP->GetSubsystem<UUISubsystem>();
	if (!UIS)
	{
		return;
	}

	UIS->ApplyGameOnlyInputMode();
	UIS->HideUI(EUIType::InGameMenu);
}

void UUW_InGameMenu::OnClickedOption()
{
	APlayerController* PC = GetOwningPlayer();
	if (!PC)
	{
		return;
	}

	ULocalPlayer* LP = PC->GetLocalPlayer();
	if (!LP)
	{
		return;
	}

	UUISubsystem* UIS = LP->GetSubsystem<UUISubsystem>();
	if (!UIS)
	{
		return;
	}

	UIS->ShowUI(EUIType::Option);
}

void UUW_InGameMenu::OnClickedExit()
{
	UKismetSystemLibrary::QuitGame(this, GetOwningPlayer(), EQuitPreference::Quit, false);
}
