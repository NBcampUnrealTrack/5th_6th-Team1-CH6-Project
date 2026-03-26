#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UW_RoomParticipantNickname.generated.h"

class UTextBlock;

UCLASS()
class BULLETANT_API UUW_RoomParticipantNickname : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void SetNickname(const FString& Nickname);

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TextNickname;
};
