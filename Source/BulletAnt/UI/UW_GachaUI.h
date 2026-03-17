#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UW_GachaUI.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnOkayButtonClicked, int32, GachaCount);

class UButton;
class UTextBlock;

UCLASS()
class BULLETANT_API UUW_GachaUI : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	
	void ResetGachaCount();
	FORCEINLINE int32 GetGachaCount() const { return GachaCount; };
	
	UPROPERTY(BlueprintAssignable)
	FOnOkayButtonClicked OnOkayButtonClicked;

protected:

	UFUNCTION()
	void HadleUpButtonClicked();

	UFUNCTION()
	void HandleDownButtonClicked();

	UFUNCTION()
	void HandleOkayButtonClicked();

	UPROPERTY(meta = (BindWidget))
	UButton* UpButton;
	
	UPROPERTY(meta = (BindWidget))
	UButton* DownButton;

	UPROPERTY(meta = (BindWidget))
	UButton* OkayButton;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* CountText;

	int32 GachaCount = 0;
};
