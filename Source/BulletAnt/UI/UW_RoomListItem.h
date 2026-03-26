#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Multiplayer/MultiplayerSubsystem.h"
#include "UW_RoomListItem.generated.h"

class UTextBlock;
class UButton;

UCLASS()
class BULLETANT_API UUW_RoomListItem : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;

public:
	void SetRoomInfo(const FRoomInfo& InRoomInfo);
	
protected:
	UFUNCTION()
	void OpenRoomInfo();

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TextRoomName;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TextCurrentPlayers;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TextMaxPlayers;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> BtnOpenRoomInfo;

	FRoomInfo RoomInfo;
};
