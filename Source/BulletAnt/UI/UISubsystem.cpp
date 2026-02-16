

#include "UI/UISubsystem.h"
#include "Blueprint/UserWidget.h"
#include "UI/UW_RootHUD.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"

UUISubsystem::UUISubsystem()
{
    static ConstructorHelpers::FObjectFinder<UUIConfig> ConfigObj(TEXT("/Game/BulletAnt/UI/DA_UIConfig.DA_UIConfig"));
    if (ConfigObj.Succeeded())
    {
        UIConfigData = ConfigObj.Object;
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
        Slot->SetSize(Layout.Size);
    }

    Slot->SetZOrder(Layout.ZOrder);
}
