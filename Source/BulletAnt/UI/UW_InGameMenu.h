// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UW_InGameMenu.generated.h"

class UButton;

UCLASS()
class BULLETANT_API UUW_InGameMenu : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

private:
	UFUNCTION()
	void OnClickedContinue();

	UFUNCTION()
	void OnClickedOption();

	UFUNCTION()
	void OnClickedExit();

protected:
	UPROPERTY(meta = (BindWidget))
	UButton* ContinueButton;

	UPROPERTY(meta = (BindWidget))
	UButton* OptionButton;

	UPROPERTY(meta = (BindWidget))
	UButton* ExitButton;
};
