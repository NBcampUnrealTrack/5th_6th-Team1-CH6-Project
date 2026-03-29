// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UW_OptionBox.generated.h"

class UButton;
class UWidgetSwitcher;

/**
 * 
 */
UCLASS()
class BULLETANT_API UUW_OptionBox : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;

private:
	UFUNCTION()
	void HandleVideoTab();

	UFUNCTION()
	void HandleAudioTab();

	UFUNCTION()
	void HandleApply();

	UFUNCTION()
	void HandleExit();

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> VideoBtn;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton>	AudioBtn;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWidgetSwitcher>	SettingSwitcher;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ApplyBtn;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton>	ExitBtn;
};
