
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "UI/UIConfig.h"
#include "UISubsystem.generated.h"

class UUW_RootHUD;
class UCanvasPanelSlot;

UCLASS()
class BULLETANT_API UUISubsystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()
	
public:
	UUISubsystem();

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void PlayerControllerChanged(APlayerController* NewPlayerController) override;

	UUserWidget* ShowUI(EUIType Type);

	template<typename TWidget>
	TWidget* ShowUI(EUIType Type)
	{
		static_assert(TIsDerivedFrom<TWidget, UUserWidget>::IsDerived, "TWidget must derive from UUserWidget");

		return Cast<TWidget>(ShowUI(Type));
	}

	void HideUI(EUIType Type);

	void ResetAllUI();

	// 입력 모드 제어
	void ApplyGameOnlyInputMode();
	void ApplyUIOnlyInputMode(UUserWidget* FocusWidget);
	void ApplyGameAndUIInputMode(UUserWidget* FocusWidget);

	void InitRootHUD();

public:
	void ApplyLayoutPreset(UCanvasPanelSlot* Slot, const FUILayoutPreset& Layout);


private:
	UPROPERTY()
	TObjectPtr<UUW_RootHUD> RootHUD;

	UPROPERTY()
	TObjectPtr<UUIConfig> UIConfigData;

	UPROPERTY()
	TMap<EUIType, TObjectPtr<UUserWidget>> SingleWidgets;
};
