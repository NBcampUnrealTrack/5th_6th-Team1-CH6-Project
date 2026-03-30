// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UW_WaveTimer.generated.h"

class UTextBlock;
class ABAGameState;
class UImage;

UCLASS()
class BULLETANT_API UUW_WaveTimer : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	void OnEnemyCount();

protected:
	void UpdateTime();

	void UpdateClockRotation(const int InitTime, const int CurrentTime);

protected:
	UPROPERTY()
	TObjectPtr<ABAGameState> CachedGameState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UTextBlock> DateBlock;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UImage> ClockImage;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UImage> HandImage;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly/*, meta = (BindWidget)*/)
	TObjectPtr<UTextBlock> EnemyCount;
};
