
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UW_TitleScreen.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTitleButtonClicked);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnJoinButtonClicked, const FText&, InIpPort);

class UButton;
class UVerticalBox;
class UEditableText;
class UOverlay;

UCLASS()
class BULLETANT_API UUW_TitleScreen : public UUserWidget
{
	GENERATED_BODY()

public:
	UUW_TitleScreen(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeConstruct() override;

public:
	void ShowLoginPanel(bool bInShow);

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

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UOverlay> LoginPanel;		// 로그인 전에 화면 입력 막는 용도
};
