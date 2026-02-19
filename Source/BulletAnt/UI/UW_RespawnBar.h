#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UW_RespawnBar.generated.h"

class UProgressBar;
class UTextBlock;

UCLASS()
class BULLETANT_API UUW_RespawnBar : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION()
	void UpdateRespawnBar(float CurrentTime, float TotalTime);

	UPROPERTY(meta = (BindWidget))
	UProgressBar* RespawnBar;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* LeftRespawnTime;
};
