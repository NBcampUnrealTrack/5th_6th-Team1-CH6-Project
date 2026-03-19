#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UW_OreCount.generated.h"

class UTextBlock;
enum class EOreType;

UCLASS()
class BULLETANT_API UUW_OreCount : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION()
	void SetOreCount(EOreType OreType, int32 OreCount);

protected:
	// 추후 변경
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TextGold;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TextMineral;
};
