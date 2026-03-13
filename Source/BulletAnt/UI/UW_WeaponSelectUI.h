
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UW_WeaponSelectUI.generated.h"

class UVerticalBox;

UCLASS()
class BULLETANT_API UUW_WeaponSelectUI : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))
	UVerticalBox* WeaponList;

protected:
	
};
