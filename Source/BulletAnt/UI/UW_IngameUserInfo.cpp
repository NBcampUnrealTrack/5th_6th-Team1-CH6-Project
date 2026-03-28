#include "UI/UW_IngameUserInfo.h"
#include "Components/TextBlock.h"
#include "Components/RetainerBox.h"

void UUW_IngameUserInfo::SetScale(float NewScale)
{
	RetainerBox->SetRenderScale(FVector2D(NewScale, NewScale));
	RetainerBox->SetRenderOpacity(NewScale);
}

void UUW_IngameUserInfo::SetColor(const FLinearColor& Color)
{
	TextNickname->SetColorAndOpacity(FSlateColor(Color));
}

void UUW_IngameUserInfo::SetLevel(int32 Level)
{
	TextLevel->SetText(FText::AsNumber(Level));
}

void UUW_IngameUserInfo::SetNickname(const FString& Nickname)
{
	TextNickname->SetText(FText::FromString(Nickname));
}