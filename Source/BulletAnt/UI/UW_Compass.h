#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UW_Compass.generated.h"

class UImage;
class UCanvasPanel;

UCLASS()
class BULLETANT_API UUW_Compass : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime);

private:
	float GetAlphaToTarget(const FVector& TargetLocation);
	void UpdatePlayerIcons(float CompassWidth);
	UImage* CreatePlayerIcon();
	void UpdateCoreIcon(float CompassWidth);

	void UpdateIconByAlpha(UImage* Icon, float CompassWidth, float Alpha);

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> ImgCompass;
	UPROPERTY()
	TWeakObjectPtr<UMaterialInstanceDynamic> CompassDynamicMaterial;
	UPROPERTY()
	TWeakObjectPtr<APlayerCameraManager> PlayerCamera;

	float CompassAngle = 180.0f;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCanvasPanel> IconCanvas;
	UPROPERTY()
	TArray<TObjectPtr<UImage>> PlayerIcons;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UTexture2D> PlayerIconTexture;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> ImgCoreIcon;

	static const FName NameCompassOffset;
	static const FName NameUVScale;
};
