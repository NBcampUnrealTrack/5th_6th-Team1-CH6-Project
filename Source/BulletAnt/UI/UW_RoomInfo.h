#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Multiplayer/MultiplayerSubsystem.h"
#include "UW_RoomInfo.generated.h"

class UTextBlock;
class UButton;
class UImage;
class UUniformGridPanel;
class UUW_RoomParticipantNickname;

UCLASS()
class BULLETANT_API UUW_RoomInfo : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;

public:
	void SetRoomInfo(const FRoomInfo& InRoomInfo);

protected:
	UFUNCTION()
	void JoinRoom();
	UFUNCTION()
	void CloseUI();

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TextRoomName;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TextCurrentPlayers;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TextMaxPlayers;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> ImgPrivate;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UUniformGridPanel> NicknameParent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<UUW_RoomParticipantNickname> NicknameUIClass;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> BtnJoin;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> BtnCancel;

	FRoomInfo RoomInfo;
};
