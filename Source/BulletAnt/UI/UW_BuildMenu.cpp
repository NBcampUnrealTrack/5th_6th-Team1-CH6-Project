

#include "UI/UW_BuildMenu.h"
#include "Components/Button.h"

UUW_BuildMenu::UUW_BuildMenu(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UUW_BuildMenu::NativeConstruct()
{
	Super::NativeConstruct();

	TurretBtn.Get()->OnClicked.AddDynamic(this, &ThisClass::OnTurretBtnClicked);
	BoxBtn.Get()->OnClicked.AddDynamic(this, &ThisClass::OnBoxBtnClicked);
	StairBtn.Get()->OnClicked.AddDynamic(this, &ThisClass::OnStairBtnClicked);
}

void UUW_BuildMenu::OnTurretBtnClicked()
{
	OnBuildMenuSelected.Broadcast(TurretRow);
}

void UUW_BuildMenu::OnBoxBtnClicked()
{
	OnBuildMenuSelected.Broadcast(BoxRow);
}

void UUW_BuildMenu::OnStairBtnClicked()
{
	OnBuildMenuSelected.Broadcast(StairRow);
}
