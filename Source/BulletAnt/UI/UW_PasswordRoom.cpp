#include "UI/UW_PasswordRoom.h"
#include "Components/EditableTextBox.h"
#include "Components/CheckBox.h"
#include "Components/Button.h"
#include "UI/UISubsystem.h"
#include "Kismet/KismetSystemLibrary.h"

void UUW_PasswordRoom::NativeConstruct()
{
	Super::NativeConstruct();

    ETBPassword->OnTextChanged.AddDynamic(this, &ThisClass::OnPasswordChanged);
    CheckBoxShowPassword->OnCheckStateChanged.AddDynamic(this, &ThisClass::HidePassword);
    BtnJoin->OnClicked.AddDynamic(this, &ThisClass::JoinRoom);
    BtnCancel->OnClicked.AddDynamic(this, &ThisClass::Cancel);

    ETBPassword->SetIsPassword(true);
    CheckBoxShowPassword->SetCheckedState(ECheckBoxState::Unchecked);
}

void UUW_PasswordRoom::SetRoomInfo(const FRoomInfo& InRoomInfo)
{
    RoomInfo = InRoomInfo;
}

void UUW_PasswordRoom::OnPasswordChanged(const FText& Text)
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
    BtnJoin->SetIsEnabled(FinalLen == 4);
}

void UUW_PasswordRoom::HidePassword(bool bHide)
{
    ETBPassword->SetIsPassword(!bHide);
}

void UUW_PasswordRoom::JoinRoom()
{
    if (RoomInfo.SearchResult.IsValid() == false)
    {
        // 유효하지 않다는 팝업 띄우기
        UKismetSystemLibrary::PrintString(GetWorld(), TEXT("SearchResult not valid"));
        return;
    }

    UMultiplayerSubsystem* MultiplayerSubsystem = GetGameInstance()->GetSubsystem<UMultiplayerSubsystem>();
	if (IsValid(MultiplayerSubsystem) == false)
		return;

    FString EnteredHash = MultiplayerSubsystem->HashPassword(ETBPassword->GetText().ToString());
    if (EnteredHash != RoomInfo.HashedPassword)
    {
        FString Str = FString::Printf(TEXT("%s / %s"), *EnteredHash, *RoomInfo.HashedPassword);
        UKismetSystemLibrary::PrintString(GetWorld(), Str);
        // 비밀번호가 틀렸다는 팝업 띄우기
        return;
    }

	MultiplayerSubsystem->JoinSession(*RoomInfo.SearchResult);
}

void UUW_PasswordRoom::Cancel()
{
    ULocalPlayer* LP = GetOwningLocalPlayer();
    if (IsValid(LP) == false)
        return;

    UUISubsystem* UISubsystem = LP->GetSubsystem<UUISubsystem>();
    if (IsValid(UISubsystem) == false)
        return;

    UISubsystem->HideUI(EUIType::PasswordRoom);
}
