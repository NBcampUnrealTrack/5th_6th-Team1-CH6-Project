
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UW_TitleScreen.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTitleButtonClicked);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnJoinButtonClicked, const FText&, InIpPort);

class UButton;
class UVerticalBox;
class UEditableText;

UCLASS()
class BULLETANT_API UUW_TitleScreen : public UUserWidget
{
	GENERATED_BODY()

public:
	UUW_TitleScreen(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeConstruct() override;

private:
	UFUNCTION()
	void OnStartBtnClicked();

	UFUNCTION()
	void OnOptionBtnClicked();

	UFUNCTION()
	void OnExitBtnClicked();

	UFUNCTION()
	void OnJoinBtnClicked();

public:
	FOnTitleButtonClicked OnStartButtonClicked;
	FOnTitleButtonClicked OnOptionButtonClicked;
	FOnTitleButtonClicked OnExitButtonClicked;

	FOnJoinButtonClicked OnJoinButtonClicked;

private:
	UPROPERTY(Meta = (BindWidget))
	TObjectPtr<UButton> StartBtn;

	UPROPERTY(Meta = (BindWidget))
	TObjectPtr<UButton> OptionBtn;

	UPROPERTY(Meta = (BindWidget))
	TObjectPtr<UButton> ExitBtn;

	UPROPERTY(Meta = (BindWidget))
	TObjectPtr<UVerticalBox> IpPortInputBox;

	UPROPERTY(Meta = (BindWidget))
	TObjectPtr<UEditableText> IpPortETxt;

	UPROPERTY(Meta = (BindWidget))
	TObjectPtr<UButton> JoinBtn;
};
