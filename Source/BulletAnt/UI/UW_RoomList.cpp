#include "UI/UW_RoomList.h"
#include "UI/UW_RoomListItem.h"
#include "Components/UniformGridPanel.h"
#include "Components/Button.h"
#include "Multiplayer/MultiplayerSubsystem.h"
#include "OnlineSessionSettings.h"

void UUW_RoomList::NativeConstruct()
{
	Super::NativeConstruct();

	BtnRefresh->OnClicked.AddDynamic(this, &ThisClass::RefreshList);
}

void UUW_RoomList::NativeDestruct()
{
	if (UpdateHandle.IsValid() == true)
	{
		UMultiplayerSubsystem* MultiplayerSubsystem = GetGameInstance()->GetSubsystem<UMultiplayerSubsystem>();
		if (IsValid(MultiplayerSubsystem) == true)
		{
			MultiplayerSubsystem->UnbindOnFindSessions(this);
		}
	}

	Super::NativeDestruct();
}

void UUW_RoomList::RefreshList()
{
	UMultiplayerSubsystem* MultiplayerSubsystem = GetGameInstance()->GetSubsystem<UMultiplayerSubsystem>();
	if (IsValid(MultiplayerSubsystem) == false)
		return;

	if (UpdateHandle.IsValid() == false)
	{
		UpdateHandle = MultiplayerSubsystem->BindOnFindSessions(FOnFindSessions::FDelegate::CreateUObject(this, &ThisClass::OnUpdateRooms));
	}

	MultiplayerSubsystem->SearchSessions(24);
}

void UUW_RoomList::OnUpdateRooms(bool bSuccessful)
{
	if (bSuccessful == false)
		return;

	UpdateList();
}

void UUW_RoomList::UpdateList()
{
	UMultiplayerSubsystem* MultiplayerSubsystem = GetGameInstance()->GetSubsystem<UMultiplayerSubsystem>();
	if (IsValid(MultiplayerSubsystem) == false)
		return;

	if (IsValid(ItemClass) == false || IsValid(ItemParent) == false)
		return;

	TArray<FRoomInfo> RoomList;
	bool bSuccess = MultiplayerSubsystem->GetRoomList(RoomList);
	if (bSuccess == false)
		return;

	for (int32 RoomIdx = 0; RoomIdx < RoomList.Num(); ++RoomIdx)
	{
		UUW_RoomListItem* NewItem = CreateWidget<UUW_RoomListItem>(this, ItemClass);
		if (IsValid(NewItem) == true)
		{
			NewItem->SetRoomInfo(RoomList[RoomIdx]);
		}

		int32 Row = RoomIdx / 3;
		int32 Col = RoomIdx % 3;
		ItemParent->AddChildToUniformGrid(NewItem, Row, Col);
	}
}
