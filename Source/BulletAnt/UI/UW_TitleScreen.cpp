#include "UI/UW_TitleScreen.h"
#include "Components/Button.h"
#include "Components/VerticalBox.h"
#include "Components/EditableText.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Player/TitlePlayerController.h"
#include "Components/Overlay.h"
#include "UI/UISubsystem.h"
#include "Multiplayer/MultiplayerSubsystem.h"
#include "UI/UW_RoomList.h"
#include "UI/UW_CreateRoom.h"

UUW_TitleScreen::UUW_TitleScreen(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UUW_TitleScreen::NativeConstruct()
{
	Super::NativeConstruct();

	BtnJoin.Get()->OnClicked.AddDynamic(this, &ThisClass::OnJoinBtnClicked);
	BtnHost.Get()->OnClicked.AddDynamic(this, &ThisClass::OnHostBtnClicked);
	BtnOption.Get()->OnClicked.AddDynamic(this, &ThisClass::OnOptionBtnClicked);
	BtnExit.Get()->OnClicked.AddDynamic(this, &ThisClass::OnExitBtnClicked);
}

void UUW_TitleScreen::ShowLoginPanel(bool bInShow)
{
	if (IsValid(LoginPanel) == true)
	{
		ESlateVisibility NewVisibility = bInShow == true ? ESlateVisibility::Visible : ESlateVisibility::Collapsed;
		LoginPanel->SetVisibility(NewVisibility);
	}
}

void UUW_TitleScreen::OnJoinBtnClicked()
{
	ULocalPlayer* LP = GetOwningLocalPlayer();
	if (IsValid(LP) == false)
		return;

	UUISubsystem* UISubsystem = LP->GetSubsystem<UUISubsystem>();
	if (IsValid(UISubsystem) == false)
		return;

	UUW_RoomList* RoomListUI = UISubsystem->ShowUI<UUW_RoomList>(EUIType::RoomList);
	if (IsValid(RoomListUI) == false)
		return;

	RoomListUI->RefreshList();
}

void UUW_TitleScreen::OnHostBtnClicked()
{
	ULocalPlayer* LP = GetOwningLocalPlayer();
	if (IsValid(LP) == false)
		return;

	UUISubsystem* UISubsystem = LP->GetSubsystem<UUISubsystem>();
	if (IsValid(UISubsystem) == false)
		return;

	UUW_CreateRoom* CreateRoomUI = UISubsystem->ShowUI<UUW_CreateRoom>(EUIType::CreateRoom);
}

void UUW_TitleScreen::OnOptionBtnClicked()
{

}

void UUW_TitleScreen::OnExitBtnClicked()
{
	UKismetSystemLibrary::QuitGame(this, GetOwningPlayer(), EQuitPreference::Quit, false);
}
