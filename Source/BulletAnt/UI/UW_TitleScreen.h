
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UW_TitleScreen.generated.h"

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
	void OnJoinBtnClicked();

	UFUNCTION()
	void OnHostBtnClicked();

	UFUNCTION()
	void OnOptionBtnClicked();

	UFUNCTION()
	void OnExitBtnClicked();

private:
	UPROPERTY(Meta = (BindWidget))
	TObjectPtr<UButton> BtnJoin;

	UPROPERTY(Meta = (BindWidget))
	TObjectPtr<UButton> BtnHost;

	UPROPERTY(Meta = (BindWidget))
	TObjectPtr<UButton> BtnOption;

	UPROPERTY(Meta = (BindWidget))
	TObjectPtr<UButton> BtnExit;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UOverlay> LoginPanel;		// 로그인 전에 화면 입력 막는 용도
};
