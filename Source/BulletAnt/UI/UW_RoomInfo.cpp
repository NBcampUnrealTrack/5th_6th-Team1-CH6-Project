#include "UI/UW_RoomInfo.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Multiplayer/MultiplayerSubsystem.h"
#include "UI/UISubsystem.h"
#include "Components/UniformGridPanel.h"
#include "UI/UW_RoomParticipantNickname.h"
#include "UI/UW_PasswordRoom.h"

void UUW_RoomInfo::NativeConstruct()
{
	Super::NativeConstruct();

	BtnJoin->OnClicked.AddDynamic(this, &ThisClass::JoinRoom);
	BtnCancel->OnClicked.AddDynamic(this, &ThisClass::CloseUI);
}

void UUW_RoomInfo::SetRoomInfo(const FRoomInfo& InRoomInfo)
{
	RoomInfo = InRoomInfo;

	TextRoomName->SetText(FText::FromString(RoomInfo.RoomName));
	TextCurrentPlayers->SetText(FText::FromString(FString::FromInt(RoomInfo.CurrentPlayers)));
	TextMaxPlayers->SetText(FText::FromString(FString::FromInt(RoomInfo.MaxPlayers)));
	ImgPrivate->SetVisibility(RoomInfo.bIsPrivate == true ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);

	const auto& Nicknames = RoomInfo.ParticipantNicknames;
	for (int32 Idx = 0; Idx < Nicknames.Num(); ++Idx)
	{
		UUW_RoomParticipantNickname* NewNicknameUI = CreateWidget<UUW_RoomParticipantNickname>(this, NicknameUIClass);
		if (IsValid(NewNicknameUI) == true)
		{
			NewNicknameUI->SetNickname(Nicknames[Idx]);
		}

		int32 Row = Idx / 4;
		int32 Col = Idx % 4;
		NicknameParent->AddChildToUniformGrid(NewNicknameUI, Row, Col);
	}
}

void UUW_RoomInfo::JoinRoom()
{
	if (RoomInfo.bIsPrivate == true)
	{
		ULocalPlayer* LP = GetOwningLocalPlayer();
		if (IsValid(LP) == false)
			return;

		UUISubsystem* UISubsystem = LP->GetSubsystem<UUISubsystem>();
		if (IsValid(UISubsystem) == false)
			return;

		UUW_PasswordRoom* PasswordRoomUI = UISubsystem->ShowUI<UUW_PasswordRoom>(EUIType::PasswordRoom);
		PasswordRoomUI->SetRoomInfo(RoomInfo);

		return;
	}

	UMultiplayerSubsystem* MultiplayerSubsystem = GetGameInstance()->GetSubsystem<UMultiplayerSubsystem>();
	if (IsValid(MultiplayerSubsystem) == false)
		return;

	if (RoomInfo.SearchResult.IsValid() == false)
	{
		// 안내 팝업 띄우기
		return;
	}

	MultiplayerSubsystem->JoinSession(*RoomInfo.SearchResult);
}

void UUW_RoomInfo::CloseUI()
{
	ULocalPlayer* LP = GetOwningLocalPlayer();
	if (IsValid(LP) == false)
		return;

	UUISubsystem* UISubsystem = LP->GetSubsystem<UUISubsystem>();
	if (IsValid(UISubsystem) == false)
		return;

	UISubsystem->HideUI(EUIType::RoomInfo);
}
