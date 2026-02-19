#include "UI/UW_GroundScanner.h"
#include "Player/BACharacter.h"
#include "Templates/SharedPointer.h"
#include "Widgets/SWidget.h"
#include "Components/CanvasPanelSlot.h"

FReply UUW_GroundScanner::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    if (InMouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton) == true)
    {
        bDragging = true;
        LastMousePos = InMouseEvent.GetScreenSpacePosition();
        return FReply::Handled().CaptureMouse(TakeWidget());
    }

    return FReply::Unhandled();
}

FReply UUW_GroundScanner::NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    if (bDragging == true)
    {
        FVector2D CurrentPos = InMouseEvent.GetScreenSpacePosition();
        FVector2D Delta = CurrentPos - LastMousePos;

        // 캐릭터 참조
        if (APlayerController* PC = GetOwningPlayer())
        {
            if (ABACharacter* Character = Cast<ABACharacter>(PC->GetPawn()))
            {
                Character->RotateScannerParent(Delta);
            }
        }

        LastMousePos = CurrentPos;
        return FReply::Handled();
    }
    return FReply::Unhandled();
}

FReply UUW_GroundScanner::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    if (bDragging == true && InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
    {
        bDragging = false;
        return FReply::Handled().ReleaseMouseCapture();
    }
    return FReply::Unhandled();
}
