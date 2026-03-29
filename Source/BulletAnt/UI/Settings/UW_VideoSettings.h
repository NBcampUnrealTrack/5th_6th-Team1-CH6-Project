// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Settings/SettingsSubsystem.h"
#include "UW_VideoSettings.generated.h"

class UUW_OptionRow;
class UUW_ComboOption;

/**
 * 
 */
UCLASS()
class BULLETANT_API UUW_VideoSettings : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;

public:
	void SyncFromSubsystem();
	void ApplyToSubsystem();

private:
	void BuildOptions();
	UUW_ComboOption* GetComboFromRow(UUW_OptionRow* Row);

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UUW_OptionRow> WindowMode;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UUW_OptionRow> Resolution;

	TArray<EBAWindowMode> WindowModeMap;
	TArray<FIntPoint> ResolutionMap;
};
