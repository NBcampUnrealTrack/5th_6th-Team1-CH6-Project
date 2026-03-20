#include "UI/UW_Compass.h"
#include "Components/Image.h"
#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Framework/BAGameState.h"
#include "GameFramework/PlayerState.h"

const FName UUW_Compass::NameCompassOffset("Offset");
const FName UUW_Compass::NameUVScale("UVScale");

void UUW_Compass::NativeConstruct()
{
	Super::NativeConstruct();

	if (IsValid(ImgCompass) == true)
	{
		CompassDynamicMaterial = ImgCompass->GetDynamicMaterial();
	}
}

void UUW_Compass::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (PlayerCamera.IsValid() == false)
	{
		APlayerController* PC = GetOwningPlayer();
		PlayerCamera = IsValid(PC) == true ? PC->PlayerCameraManager : nullptr;
	}

	if (CompassDynamicMaterial.IsValid() == true && PlayerCamera.IsValid() == true)
	{
		float CurrentYaw = PlayerCamera->GetCameraRotation().Yaw;
		// 텍스처에서 정중앙에 N이 오도록 함.
		// NormalizedYaw  = Yaw가 0일 때, 0.5
		float NormalizedYaw = (CurrentYaw + 180.0f) / 360.0f;
		float UVScale = CompassAngle / 360.0f;
		float FinalOffset = NormalizedYaw - (UVScale * 0.5f);
		CompassDynamicMaterial->SetScalarParameterValue(NameUVScale, UVScale);
		CompassDynamicMaterial->SetScalarParameterValue(NameCompassOffset, FinalOffset);
	}

    if (IsValid(ImgCompass) == true)
    {
        float CompassWidth = ImgCompass->GetCachedGeometry().GetLocalSize().X;
        UpdatePlayerIcons(CompassWidth);
        UpdateCoreIcon(CompassWidth);
    }
}

float UUW_Compass::GetAlphaToTarget(const FVector& TargetLocation)
{
	APawn* OwnerPawn = GetOwningPlayerPawn();
	APlayerController* PC = GetOwningPlayer();
	if (IsValid(OwnerPawn) == false || IsValid(PC) == false)
		return 0.5f;

	FVector DirToTarget = (TargetLocation - OwnerPawn->GetActorLocation()).GetSafeNormal();
	FRotator LookAtRot = DirToTarget.Rotation();
	float RelativeYaw = FMath::UnwindDegrees(LookAtRot.Yaw - PC->GetControlRotation().Yaw);
	float HalfCompassAngle = CompassAngle * 0.5f;
	float Alpha = (RelativeYaw + HalfCompassAngle) / CompassAngle;

	return Alpha; // 0이면 왼쪽 끝, 0.5면 중앙, 1이면 오른쪽 끝 <- 해당 범위에 안 들어오면 표시 X
}

void UUW_Compass::UpdatePlayerIcons(float CompassWidth)
{
    APlayerController* PC = GetOwningPlayer();
    if (IsValid(PC) == false)
        return;

    const TArray<APlayerState*>& AllPlayers = GetWorld()->GetGameState()->PlayerArray;
    int32 ActiveIconIdx = 0;
    for (APlayerState* PS : AllPlayers)
    {
        if (IsValid(PS) == false || PS == PC->PlayerState)
            continue;

        APawn* TargetPawn = PS->GetPawn();
        if (IsValid(TargetPawn) == false)
            continue;

        if (ActiveIconIdx >= PlayerIcons.Num())
        {
            UImage* NewIcon = CreatePlayerIcon();
            if (IsValid(NewIcon) == false)
                continue;       // 아이콘 생성 실패

            PlayerIcons.Add(NewIcon);
        }

        float Alpha = GetAlphaToTarget(TargetPawn->GetActorLocation());
        UpdateIconByAlpha(PlayerIcons[ActiveIconIdx], CompassWidth, Alpha);
        ++ActiveIconIdx;
    }

    for (int32 Idx = ActiveIconIdx; Idx < PlayerIcons.Num(); ++Idx)
    {
        if (PlayerIcons[Idx]->GetVisibility() != ESlateVisibility::Collapsed)
        {
            PlayerIcons[Idx]->SetVisibility(ESlateVisibility::Collapsed);
        }
    }
}

UImage* UUW_Compass::CreatePlayerIcon()
{
	if (IsValid(WidgetTree) == false || IsValid(IconCanvas) == false)
		return nullptr;

	UImage* NewIcon = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass());
	NewIcon->SetBrushFromTexture(PlayerIconTexture);

	UCanvasPanelSlot* NewSlot = IconCanvas->AddChildToCanvas(NewIcon);
	if (IsValid(NewSlot) == true)
	{
		NewSlot->SetAnchors(FAnchors(0.5f, 0.0f));
		NewSlot->SetAlignment(FVector2D(0.5f, 1.0f));
		NewSlot->SetAutoSize(true);
	}

	return NewIcon;
}

void UUW_Compass::UpdateCoreIcon(float CompassWidth)
{
    float Alpha = GetAlphaToTarget(FVector(0.0f, 0.0f, 0.0f));
    UpdateIconByAlpha(ImgCoreIcon, CompassWidth, Alpha);
}

void UUW_Compass::UpdateIconByAlpha(UImage* Icon, float CompassWidth, float Alpha)
{
    if (IsValid(Icon) == false)
        return;

    if (Alpha >= 0.0f && Alpha <= 1.0f)
    {
        Icon->SetVisibility(ESlateVisibility::HitTestInvisible);
        if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(Icon->Slot))
        {
            float NewX = Alpha * CompassWidth;
            CanvasSlot->SetPosition(FVector2D(NewX, 10.0f));
        }
    }
    else
    {
        Icon->SetVisibility(ESlateVisibility::Collapsed);
    }
}
