
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Building/BuildingRow.h"
#include "UW_BuildMenu.generated.h"

class UButton;
class UWrapBox;
class UDataTable;
class UUW_BuildingIcon;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBuildMenuSelected, FName, BuildingRow);

UCLASS()
class BULLETANT_API UUW_BuildMenu : public UUserWidget
{
	GENERATED_BODY()

public:
	UUW_BuildMenu(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeConstruct() override;

private:
	UFUNCTION()
	void OnTurretBtnClicked();

	UFUNCTION()
	void OnBuildingBtnClicked();

	UFUNCTION()
	void OnEtcBtnClicked();

	UFUNCTION()
	void HandleBuildingIconClicked(FName BuildingRowName);

	void SetCurrentCategory(EBuildCategory NewCategory);
	void RefreshBuildingList();

public:
	FOnBuildMenuSelected OnBuildMenuSelected;

private:
	UPROPERTY(Meta = (BindWidget))
	TObjectPtr<UButton> TurretBtn;

	UPROPERTY(Meta = (BindWidget))
	TObjectPtr<UButton> BuildingBtn;

	UPROPERTY(Meta = (BindWidget))
	TObjectPtr<UButton> EtcBtn;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWrapBox> BuildingListWrapBox;

	UPROPERTY(EditDefaultsOnly, Category = "Build")
	TObjectPtr<UDataTable> BuildingDataTable;

	UPROPERTY(EditDefaultsOnly, Category = "Build")
	TSubclassOf<UUW_BuildingIcon> BuildingIconWidgetClass;

	UPROPERTY()
	EBuildCategory CurrentCategory = EBuildCategory::Turret;
};
