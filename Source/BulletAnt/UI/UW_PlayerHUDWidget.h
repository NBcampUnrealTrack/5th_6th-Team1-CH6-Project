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
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	void InitPlayerHUD();

	UFUNCTION()
	void AddWeaponLog(UWeaponDataAsset* InData);

	UPROPERTY()
	TWeakObjectPtr<ABACharacter> OwnerCharacter;

	void ShowAmmoText();
	void HideAmmoText();

	UFUNCTION()
	void UpdateAmmo(float Current, float Max);

	void SetAutoImage(bool bIsFullAuto);
	void SetCrossHairImage(bool bIsADS);

protected:
	UFUNCTION()
	void UpdateHealth(float Current, float Max);

	UFUNCTION()
	void UpdateEXP(float Current, float Max);

	UFUNCTION()
	void UpdateLevel(float Current, float OldLevel);

	UPROPERTY(meta = (BindWidget))
	UProgressBar* ActualHealthBar;

	UPROPERTY(meta = (BindWidget))
	UProgressBar* HealthBar;

	float DisplayedHealthPercent = 1.f;
	float TargetHealthPercent = 1.f;
	float HealthLerpSpeed = 2.f;

	UPROPERTY(meta = (BindWidget))
	UProgressBar* EXPBar;

	float DisplayedEXPPercent = 0.f;
	float TargetEXPPercent = 0.f;
	float EXPLerpSpeed = 2.f;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* CurrentLevelText;

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
	UImage* CurrentCrossHair;

	UPROPERTY(EditAnywhere)
	UTexture2D* NormalCrossHair;

	UPROPERTY(EditAnywhere)
	UTexture2D* ADSCrossHair;

	UPROPERTY(meta = (BindWidget))
	UVerticalBox* WeaponLogBox;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UUW_WeaponLog> WeaponLogClass;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UUW_OreCount> OreCountUI;
};
