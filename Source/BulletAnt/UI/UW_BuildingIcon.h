
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UW_BuildingIcon.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBuildingIconClicked, FName, BuildingRowName);

class UButton;
class UTextBlock;
class UImage;
class UTexture2D;

UCLASS()
class BULLETANT_API UUW_BuildingIcon : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	void SetupBuildingIcon(FName InRowName, const FText& InDisplayName, UTexture2D* InIconTexture);

protected:
	UFUNCTION()
	void OnButtonClicked();

public:
	UPROPERTY(BlueprintAssignable)
	FOnBuildingIconClicked OnBuildingIconClicked;

private:

	UPROPERTY()
	FName BuildingRowName;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> IconButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> BuildingIconImage;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> BuildingNameText;
};
