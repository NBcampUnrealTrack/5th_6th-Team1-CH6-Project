#include "UI/UW_RoomParticipantNickname.h"
#include "Components/TextBlock.h"

void UUW_RoomParticipantNickname::SetNickname(const FString& Nickname)
{
	TextNickname->SetText(FText::FromString(Nickname));
}
