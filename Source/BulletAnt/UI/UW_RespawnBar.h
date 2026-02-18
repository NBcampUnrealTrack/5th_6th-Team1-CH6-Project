#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UW_RespawnBar.generated.h"

class UProgressBar

UCLASS()
class BULLETANT_API UUW_RespawnBar : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))
	UProgressBar* RespawnBar;
};
