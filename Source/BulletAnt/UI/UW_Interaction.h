#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Common/BAItemInterface.h"
#include "UW_Interaction.generated.h"

class UVerticalBox;
class UUW_InteractionItem;

UCLASS()
class BULLETANT_API UUW_Interaction : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void SetInteractionOptions(const TArray<FInteractionOption>& InOptions);

	UFUNCTION(BlueprintCallable)
	void ClearInteraction();

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UVerticalBox> VerticalBox_Options;

	UPROPERTY(EditDefaultsOnly, Category = "Interaction")
	TSubclassOf<UUW_InteractionItem> InteractionItemClass;
};