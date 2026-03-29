// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UW_OptionRow.generated.h"

class UTextBlock;
class UNamedSlot;
class UWidget;

/**
 * 
 */
UCLASS()
class BULLETANT_API UUW_OptionRow : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void SetOptionNameText(const FText& InText);

	float GetOptionValue(float DefaultValue = 0) const;

	UWidget* GetValueWidget() const;

private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> OptionName;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UNamedSlot> RowSlot;
};
