#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Multiplayer/MultiplayerSubsystem.h"
#include "UW_PasswordRoom.generated.h"

class UEditableTextBox;
class UCheckBox;
class UButton;
class UOverlay;

UCLASS()
class BULLETANT_API UUW_PasswordRoom : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	void SetRoomInfo(const FRoomInfo& InRoomInfo);

	UFUNCTION()
	void OnPasswordChanged(const FText& Text);
	UFUNCTION()
	void HidePassword(bool bHide);
	UFUNCTION()
	void JoinRoom();
	UFUNCTION()
	void Cancel();

	void ShowLoadingPanel(bool bShow);
	
protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEditableTextBox> ETBPassword;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCheckBox> CheckBoxShowPassword;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UOverlay> LoadingPanel;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> BtnJoin;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> BtnCancel;

	FDelegateHandle JoinHandle;

	FRoomInfo RoomInfo;
};
