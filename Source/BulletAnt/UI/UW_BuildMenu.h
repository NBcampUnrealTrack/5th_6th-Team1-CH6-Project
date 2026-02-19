
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UW_BuildMenu.generated.h"

class UButton;

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
	void OnBoxBtnClicked();

	UFUNCTION()
	void OnStairBtnClicked();

public:
	FOnBuildMenuSelected OnBuildMenuSelected;

private:
	UPROPERTY(Meta = (BindWidget))
	TObjectPtr<UButton> TurretBtn;

	UPROPERTY(Meta = (BindWidget))
	TObjectPtr<UButton> BoxBtn;

	UPROPERTY(Meta = (BindWidget))
	TObjectPtr<UButton> StairBtn;

	// 임시 row 이름들(나중에 제거)
	UPROPERTY(EditDefaultsOnly, Category = "Build")
	FName TurretRow = TEXT("TestTurret");

	UPROPERTY(EditDefaultsOnly, Category = "Build")
	FName BoxRow = TEXT("TestBox");

	UPROPERTY(EditDefaultsOnly, Category = "Build")
	FName StairRow = TEXT("TestStair");
};
