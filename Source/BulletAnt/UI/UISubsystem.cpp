

#include "UI/UISubsystem.h"
#include "Blueprint/UserWidget.h"
#include "UI/UW_RootHUD.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "GameFramework/PlayerController.h"

UUISubsystem::UUISubsystem()
{
}

void UUISubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    static const TCHAR* UIConfigPath = TEXT("/Game/BulletAnt/UI/DA_UIConfig.DA_UIConfig");
    UIConfigData = LoadObject<UUIConfig>(nullptr, UIConfigPath);

    if (!UIConfigData)
    {
        UE_LOG(LogTemp, Error, TEXT("UIConfig load failed: %s"), UIConfigPath);
    }
}

void UUISubsystem::PlayerControllerChanged(APlayerController* NewPlayerController)
{
    Super::PlayerControllerChanged(NewPlayerController);

    ResetAllUI();
    InitRootHUD();
}

UUserWidget* UUISubsystem::ShowUI(EUIType Type)
{
    // 이미 생성된 UI가 있을 경우
    if (TObjectPtr<UUserWidget>* Found = SingleWidgets.Find(Type))
    {
        (*Found)->SetVisibility(ESlateVisibility::Visible);
        return Found->Get();
    }

    // UI를 생성해야 하는 경우
    APlayerController* PC = GetLocalPlayer()->GetPlayerController(GetWorld());
    if (!PC)
    {
        return nullptr;
    }

    FUIInfo WidgetInfo;
    if (!UIConfigData->GetInfo(Type, WidgetInfo))
    {
        return nullptr;
    }

    UUserWidget* WidgetInstance = CreateWidget<UUserWidget>(PC, WidgetInfo.WidgetClass);
    if (!WidgetInstance)
    {
        return nullptr;
    }

    if (!RootHUD)
    {
        InitRootHUD();
    }
    UCanvasPanelSlot* Slot = RootHUD->GetRootCanvas()->AddChildToCanvas(WidgetInstance);
    if (!Slot)
    {
        return nullptr;
    }
    ApplyLayoutPreset(Slot, WidgetInfo.Layout);

    SingleWidgets.Add(Type, WidgetInstance);

    return WidgetInstance;
}

void UUISubsystem::HideUI(EUIType Type)
{
    if (TObjectPtr<UUserWidget>* Found = SingleWidgets.Find(Type))
    {
        (*Found)->SetVisibility(ESlateVisibility::Collapsed);
    }
}

void UUISubsystem::ResetAllUI()
{
    for (auto& Pair : SingleWidgets)
    {
        if (IsValid(Pair.Value))
        {
            Pair.Value->RemoveFromParent();
        }
    }
    SingleWidgets.Empty();
}

void UUISubsystem::ApplyGameOnlyInputMode()
{
    APlayerController* PC = GetLocalPlayer()->GetPlayerController(GetWorld());
    if (!PC)
    {
        return;
    }

    FInputModeGameOnly InputMode;
    PC->SetInputMode(InputMode);
    PC->SetShowMouseCursor(false);
    PC->SetIgnoreLookInput(false);
}

void UUISubsystem::ApplyUIOnlyInputMode(UUserWidget* FocusWidget)
{
    APlayerController* PC = GetLocalPlayer()->GetPlayerController(GetWorld());
    if (!PC || !FocusWidget)
    {
        return;
    }

    FInputModeUIOnly InputMode;
    InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);

    PC->SetInputMode(InputMode);
    PC->SetShowMouseCursor(true);
    PC->SetIgnoreLookInput(true);
}

void UUISubsystem::ApplyGameAndUIInputMode(UUserWidget* FocusWidget)
{
    APlayerController* PC = GetLocalPlayer()->GetPlayerController(GetWorld());
    if (!PC || !FocusWidget)
    {
        return;
    }

    FInputModeGameAndUI InputMode;
    InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
    InputMode.SetHideCursorDuringCapture(false);

    PC->SetInputMode(InputMode);
    PC->SetShowMouseCursor(true);
    PC->SetIgnoreLookInput(true);
}

void UUISubsystem::InitRootHUD()
{
    APlayerController* PC = GetLocalPlayer()->GetPlayerController(GetWorld());
    if (!PC)
    {
        return;
    }

    if (IsValid(RootHUD))
    {
        if (!RootHUD->IsInViewport())
        {
            RootHUD->AddToViewport();
        }
        return;
    }

    FUIInfo RootInfo;
    if (!UIConfigData->GetInfo(EUIType::Root, RootInfo))
    {
        return;
    }

    RootHUD = CreateWidget<UUW_RootHUD>(PC, RootInfo.WidgetClass);
    if (RootHUD == nullptr)
    {
        return;
    }
    RootHUD->AddToViewport();
}

void UUISubsystem::ApplyLayoutPreset(UCanvasPanelSlot* Slot, const FUILayoutPreset& Layout)
{
    Slot->SetAutoSize(Layout.bAutoSize);
    Slot->SetAnchors(Layout.Anchors);
    Slot->SetAlignment(Layout.Alignment);
    Slot->SetPosition(Layout.Position);

    if (!Layout.bAutoSize)
    {
        if (Layout.Anchors.Minimum == FVector2D(0.f, 0.f) &&
            Layout.Anchors.Maximum == FVector2D(1.f, 1.f))
        {
            // 풀스크린
            Slot->SetOffsets(FMargin(0.f));
        }
        else
        {
            // 일반 배치
            Slot->SetPosition(Layout.Position);

            if (!Layout.bAutoSize)
            {
                Slot->SetSize(Layout.Size);
            }
        }
    }

    Slot->SetZOrder(Layout.ZOrder);
}
