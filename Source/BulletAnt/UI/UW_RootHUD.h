
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/CanvasPanel.h"
#include "UW_RootHUD.generated.h"

UCLASS()
class BULLETANT_API UUW_RootHUD : public UUserWidget
{
	GENERATED_BODY()
public:
	inline UCanvasPanel* GetRootCanvas() const { return RootCanvas.Get(); }

private:
	UPROPERTY(Meta = (BindWidget))
	TObjectPtr<UCanvasPanel> RootCanvas;
};
