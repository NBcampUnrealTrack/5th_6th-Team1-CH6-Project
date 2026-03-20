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

UCLASS()
class BULLETANT_API UUW_PlayerHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	UPROPERTY()
	TWeakObjectPtr<ABACharacter> OwnerCharacter;

	UFUNCTION()
	void AddWeaponLog(UWeaponDataAsset* InData);

protected:
	UFUNCTION()
	void UpdateHealth(float Current, float Max);

	UFUNCTION()
	void UpdateAmmo(float Current, float Max);

	UPROPERTY(meta = (BindWidget))
	UProgressBar* HealthBar;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* AmmoText;

	UPROPERTY(meta = (BindWidget))
	UVerticalBox* WeaponLogBox;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UUW_WeaponLog> WeaponLogClass;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UUW_OreCount> OreCountUI;
};
