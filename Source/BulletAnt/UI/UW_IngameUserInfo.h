#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UW_IngameUserInfo.generated.h"

class UTextBlock;
class URetainerBox;

UCLASS()
class BULLETANT_API UUW_IngameUserInfo : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetScale(float NewScale);

	void SetColor(const FLinearColor& Color);
	void SetLevel(int32 Level);
	void SetNickname(const FString& Nickname);
	
protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<URetainerBox> RetainerBox;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TextLevel;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TextNickname;
};
