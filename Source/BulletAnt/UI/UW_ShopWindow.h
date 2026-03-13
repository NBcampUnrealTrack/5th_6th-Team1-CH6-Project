#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UW_ShopWindow.generated.h"

class UButton;
class UVerticalBox;
class UUW_WeaponButton;
class UUW_GachaUI;
class ABaseWeapon;
class ABaseShop;
class USoundBase;
class UUW_WeaponSelectUI;
class UUW_WeaponInfoUI;

UCLASS()
class BULLETANT_API UUW_ShopWindow : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION()
	void OnClickGachaButton(int32 Count);

	UFUNCTION()
	void OnClickEndButton();

	UFUNCTION()
	void SetupWeaponButton();

	UFUNCTION()
	void InitShopUI(ABaseShop* InShop);

	UFUNCTION()
	void HandleWeaponSelected(TSubclassOf<ABaseWeapon> WeaponClass);

	UFUNCTION()
	void CreateWeaponButton(const TArray<TSubclassOf<ABaseWeapon>>& Weapons);

	UFUNCTION()
	void RequestEquipWeapon();

protected:
	virtual void NativeConstruct() override;
	
	UPROPERTY(meta = (BindWidget))
	UUW_GachaUI* GachaUI;

	UPROPERTY(meta = (BindWidget))
	UUW_WeaponSelectUI* WeaponSelectUI;

	UPROPERTY(meta = (BindWidget))
	UUW_WeaponInfoUI* WeaponInfoUI;

	UPROPERTY(meta = (BindWidget))
	UButton* EndButton;

	UPROPERTY()
	TWeakObjectPtr<ABaseShop> CachedShop;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UUW_WeaponButton> WeaponButtonClass;

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	USoundBase* BuySuccessSound;

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	USoundBase* BuyFailedSound;
};
