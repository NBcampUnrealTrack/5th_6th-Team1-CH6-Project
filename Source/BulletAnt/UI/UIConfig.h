
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Blueprint/UserWidget.h"
#include "UIConfig.generated.h"

UENUM()
enum class EUIType : uint8
{
    Root,
    Title,
    BuildMenu,
    RespawnBar,
    GroundScanner,
    WaveTimer,
    Shop,
    Compass,
};

USTRUCT()
struct FUILayoutPreset
{
    GENERATED_BODY()

    UPROPERTY(EditDefaultsOnly)
    uint8 bAutoSize : 1 = true;

    UPROPERTY(EditDefaultsOnly)
    FAnchors Anchors = FAnchors(0.5f, 0.5f);

    UPROPERTY(EditDefaultsOnly)
    FVector2D Alignment = FVector2D(0.5f, 0.5f);

    UPROPERTY(EditDefaultsOnly)
    FVector2D Position = FVector2D::ZeroVector;

    UPROPERTY(EditDefaultsOnly, meta = (EditCondition = "!bAutoSize"))
    FVector2D Size = FVector2D(400.f, 300.f);

    UPROPERTY(EditDefaultsOnly)
    int32 ZOrder = 0;
};

USTRUCT()
struct FUIInfo
{
    GENERATED_BODY()

    UPROPERTY(EditDefaultsOnly)
    TSubclassOf<UUserWidget> WidgetClass;

    UPROPERTY(EditDefaultsOnly)
    FUILayoutPreset Layout;
};

UCLASS()
class BULLETANT_API UUIConfig : public UDataAsset
{
	GENERATED_BODY()
	
public:
    bool GetInfo(EUIType Type, FUIInfo& OutInfo) const;

private:
    UPROPERTY(EditDefaultsOnly)
    TMap<EUIType, FUIInfo> UIData;
};
