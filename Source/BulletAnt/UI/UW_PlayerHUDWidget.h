#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UW_PlayerHUDWidget.generated.h"

class ABACharacter;
class UProgressBar;
class UTextBlock;
class UVerticalBox;
class UUW_WeaponLog;
class UUW_OreCount;
class UImage;

UCLASS()
class BULLETANT_API UUW_PlayerHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	void InitPlayerHUD();

	UFUNCTION()
	void AddWeaponLog(UWeaponDataAsset* InData);

	UPROPERTY()
	TWeakObjectPtr<ABACharacter> OwnerCharacter;

	void ShowAmmoText();
	void HideAmmoText();

	void SetAutoImage(bool bIsFullAuto);

protected:
	UFUNCTION()
	void UpdateHealth(float Current, float Max);

	UFUNCTION()
	void UpdateAmmo(float Current, float Max);

	UPROPERTY(meta = (BindWidget))
	UProgressBar* HealthBar;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* TotalAmmoText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* CurrentAmmoText;

	UPROPERTY(EditAnywhere)
	UTexture2D* SingleShotImage;

	UPROPERTY(EditAnywhere)
	UTexture2D* FullAutoShotImage;

	UPROPERTY(meta = (BindWidget))
	UImage* CurrentShotImage;

	UPROPERTY(meta = (BindWidget))
	UVerticalBox* WeaponLogBox;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UUW_WeaponLog> WeaponLogClass;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UUW_OreCount> OreCountUI;
};
