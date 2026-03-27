#include "UI/UW_RoomListItem.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "UI/UISubsystem.h"
#include "UI/UW_RoomInfo.h"

void UUW_RoomListItem::NativeConstruct()
{
	Super::NativeConstruct();

	BtnOpenRoomInfo->OnClicked.AddDynamic(this, &ThisClass::OpenRoomInfo);
}

void UUW_RoomListItem::SetRoomInfo(const FRoomInfo& InRoomInfo)
{
	RoomInfo = InRoomInfo;

	TextRoomName->SetText(FText::FromString(RoomInfo.RoomName));
	TextCurrentPlayers->SetText(FText::FromString(FString::FromInt(RoomInfo.CurrentPlayers)));
	TextMaxPlayers->SetText(FText::FromString(FString::FromInt(RoomInfo.MaxPlayers)));
	ImgPrivate->SetVisibility(RoomInfo.bIsPrivate == true ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
}

void UUW_RoomListItem::OpenRoomInfo()
{
	ULocalPlayer* LP = GetOwningLocalPlayer();
	if (IsValid(LP) == false)
		return;

	UUISubsystem* UISubsystem = LP->GetSubsystem<UUISubsystem>();
	if (IsValid(UISubsystem) == false)
		return;

	UUW_RoomInfo* RoomInfoUI = UISubsystem->ShowUI<UUW_RoomInfo>(EUIType::RoomInfo);
	RoomInfoUI->SetRoomInfo(RoomInfo);
}
