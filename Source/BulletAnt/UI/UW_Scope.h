#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UW_Scope.generated.h"

class UBorder;

UCLASS()
class BULLETANT_API UUW_Scope : public UUserWidget
{
	GENERATED_BODY()

public:
	void InitScope(UTextureRenderTarget2D* InRT);

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Scope")
	UMaterialInterface* ScopeMaterial;

	UPROPERTY()
	UMaterialInstanceDynamic* CachedMID;

	UPROPERTY(meta = (BindWidget))
	UBorder* Render;
};
