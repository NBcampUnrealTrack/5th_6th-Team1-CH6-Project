#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UW_WeaponButton.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeaponButtonClicked, TSubclassOf<ABaseWeapon>, InWeaponClass);

class UButton;
class ABaseWeapon;
class UTextBlock;

UCLASS()
class BULLETANT_API UUW_WeaponButton : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetWeaponName(const FText& InName);

	UPROPERTY(BlueprintAssignable)
	FOnWeaponButtonClicked OnClickWeapon;
	
	UPROPERTY()
	TSubclassOf<ABaseWeapon> WeaponClass;

	

protected:
	virtual void NativeConstruct() override;

	UFUNCTION()
	void HandleWeaponButtonClick();

	UPROPERTY(meta = (BindWidget))
	UButton* Button;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* WeaponName;
};
