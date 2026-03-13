#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UW_WeaponInfoUI.generated.h"

class UButton;
class UProgressBar;
class URangedWeaponDataAsset;
class UTextBlock;
class ABaseWeapon;

UCLASS()
class BULLETANT_API UUW_WeaponInfoUI : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	void InitWeaponInfoUI(TSubclassOf<ABaseWeapon> InWeaponClass, URangedWeaponDataAsset* InData);

	UPROPERTY(meta = (BindWidget))
	UButton* EquipButton;

	TSubclassOf<ABaseWeapon> WeaponClass;

protected:

	UPROPERTY(meta = (BindWidget))
	UProgressBar* DamageBar;

	UPROPERTY(meta = (BindWidget))
	UProgressBar* AccuracyBar;

	UPROPERTY(meta = (BindWidget))
	UProgressBar* ReloadBar;

	UPROPERTY(meta = (BindWidget))
	UProgressBar* RPMBar;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* WeaponName;

	
};
