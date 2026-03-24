#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Common/BAItemInterface.h"
#include "UW_InteractionItem.generated.h"

class UTextBlock;

UCLASS()
class BULLETANT_API UUW_InteractionItem : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void SetData(const FInteractionOption& InOption);

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_Key;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_Label;
};