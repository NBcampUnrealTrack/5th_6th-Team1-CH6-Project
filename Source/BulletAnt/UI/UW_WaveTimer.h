// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UW_WaveTimer.generated.h"

class UTextBlock;
class ABAGameState;

UCLASS()
class BULLETANT_API UUW_WaveTimer : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

protected:
	void UpdateTime();
	void SetColor(float alpha);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UTextBlock> TimeBlock;

	UPROPERTY(EditDefaultsOnly, Category = "UI Settings")
	FLinearColor StartColor = FLinearColor::Green;

	UPROPERTY(EditDefaultsOnly, Category = "UI Settings")
	FLinearColor MidColor = FLinearColor::Yellow;

	UPROPERTY(EditDefaultsOnly, Category = "UI Settings")
	FLinearColor EndColor = FLinearColor::Red;

	UPROPERTY()
	TObjectPtr<ABAGameState> CachedGameState;
};
