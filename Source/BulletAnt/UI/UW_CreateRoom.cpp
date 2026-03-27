#include "UI/UW_CreateRoom.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Components/Slider.h"
#include "Components/CheckBox.h"
#include "UI/UISubsystem.h"
#include "Multiplayer/MultiplayerSubsystem.h"

void UUW_CreateRoom::NativeConstruct()
{
	Super::NativeConstruct();

    ETBRoomName->OnTextChanged.AddDynamic(this, &ThisClass::OnRoomNameChanged);
	SliderMaxPlayers->OnValueChanged.AddDynamic(this, &ThisClass::OnMaxPlayersValueChanged);
	ETBPassword->OnTextChanged.AddDynamic(this, &ThisClass::OnPasswordChanged);
    CheckBoxShowPassword->OnCheckStateChanged.AddDynamic(this, &ThisClass::HidePassword);
	BtnCreate->OnClicked.AddDynamic(this, &ThisClass::CreateRoom);
	BtnCancel->OnClicked.AddDynamic(this, &ThisClass::Cancel);

    ETBPassword->SetIsPassword(true);
    CheckBoxShowPassword->SetCheckedState(ECheckBoxState::Unchecked);
}

void UUW_CreateRoom::OnRoomNameChanged(const FText& Text)
{
    FString Str = Text.ToString();
    if (Str.Len() > 20)
    {
        Str = Str.Left(20);
        ETBRoomName->SetText(FText::FromString(Str));
    }
}

void UUW_CreateRoom::OnMaxPlayersValueChanged(float Value)
{
	int32 IntValue = FMath::RoundToInt(Value);
	TextMaxPlayers->SetText(FText::AsNumber(IntValue));
}

void UUW_CreateRoom::OnPasswordChanged(const FText& Text)
{
    FString CurrentStr = Text.ToString();
    FString FilteredStr = "";

    for (int32 Idx = 0; Idx < CurrentStr.Len(); ++Idx)
    {
        if (FChar::IsDigit(CurrentStr[Idx]))
        {
            FilteredStr.AppendChar(CurrentStr[Idx]);
        }
    }

    if (FilteredStr.Len() > 4)
    {
        FilteredStr = FilteredStr.Left(4);
    }

    if (CurrentStr != FilteredStr)
    {
        ETBPassword->SetText(FText::FromString(FilteredStr));
    }

    int32 FinalLen = FilteredStr.Len();
    BtnCreate->SetIsEnabled(FinalLen == 0 || FinalLen == 4);
}

void UUW_CreateRoom::HidePassword(bool bHide)
{
    ETBPassword->SetIsPassword(!bHide);
}

void UUW_CreateRoom::CreateRoom()
{
    UMultiplayerSubsystem* MultiplayerSubSystem = GetGameInstance()->GetSubsystem<UMultiplayerSubsystem>();
    if (IsValid(MultiplayerSubSystem) == true)
    {
        FRoomSetting RoomSetting;
        RoomSetting.RoomName = ETBRoomName->GetText().ToString();
        RoomSetting.MaxPlayers = FMath::RoundToInt(SliderMaxPlayers->GetValue());
        RoomSetting.Password = ETBPassword->GetText().ToString();
        RoomSetting.bIsPrivate = RoomSetting.Password.IsEmpty() == false;

        MultiplayerSubSystem->ServerTravelToLobby(RoomSetting);
    }
}

void UUW_CreateRoom::Cancel()
{
    ULocalPlayer* LP = GetOwningLocalPlayer();
    if (IsValid(LP) == false)
        return;

    UUISubsystem* UISubsystem = LP->GetSubsystem<UUISubsystem>();
    if (IsValid(UISubsystem) == false)
        return;

    UISubsystem->HideUI(EUIType::CreateRoom);
}
