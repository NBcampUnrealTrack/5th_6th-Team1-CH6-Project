#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Weapon/Data/WeaponDataAsset.h"
#include "UW_WeaponLog.generated.h"

class UTextBlock;

UCLASS()
class BULLETANT_API UUW_WeaponLog : public UUserWidget
{
	GENERATED_BODY()

public:
	void ShowWeaponLog(UWeaponDataAsset* InData);

protected:
	void RemoveLog();

	UPROPERTY(meta = (BindWidget))
	UTextBlock* WeaponName;

	FTimerHandle RemoveTimer;
	
};
