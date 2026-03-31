#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UW_Loading.generated.h"

class UOverlay;

UCLASS()
class BULLETANT_API UUW_Loading : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void ShowLoadingPanel(bool bShow);

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UOverlay> LoadingPanel;
};
