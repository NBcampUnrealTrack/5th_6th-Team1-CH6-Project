#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UW_GameOver.generated.h"

class UButton;
class UTextBlock;

UCLASS()
class BULLETANT_API UUW_GameOver : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	void InitText();

private:
	UFUNCTION()
	void GoToLobby();
	UFUNCTION()
	void GoToMain();
	
protected:
	UPROPERTY(meta = (BindWidget))
	UButton* ToTitleButton;

	UPROPERTY(meta = (BindWidget))
	UButton* ToLobbyButton;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* LevelText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* KillCountText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* DamageText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* CollectedResourceText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* DaysText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* BuildingText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* WeaponText;

};
