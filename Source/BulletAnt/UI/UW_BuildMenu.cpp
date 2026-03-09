

#include "UI/UW_BuildMenu.h"
#include "UI/UW_BuildingIcon.h"
#include "Components/Button.h"
#include "Components/WrapBox.h"
#include "Engine/DataTable.h"
#include "Engine/Texture2D.h"

UUW_BuildMenu::UUW_BuildMenu(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UUW_BuildMenu::NativeConstruct()
{
	Super::NativeConstruct();

	if (IsValid(TurretBtn))
	{
		TurretBtn->OnClicked.AddDynamic(this, &ThisClass::OnTurretBtnClicked);
	}

	if (IsValid(BuildingBtn))
	{
		BuildingBtn->OnClicked.AddDynamic(this, &ThisClass::OnBuildingBtnClicked);
	}

	if (IsValid(EtcBtn))
	{
		EtcBtn->OnClicked.AddDynamic(this, &ThisClass::OnEtcBtnClicked);
	}

	RefreshBuildingList();

}

void UUW_BuildMenu::OnTurretBtnClicked()
{
	SetCurrentCategory(EBuildCategory::Turret);
}

void UUW_BuildMenu::OnBuildingBtnClicked()
{
	SetCurrentCategory(EBuildCategory::Building);
}

void UUW_BuildMenu::OnEtcBtnClicked()
{
	SetCurrentCategory(EBuildCategory::Etc);
}

void UUW_BuildMenu::HandleBuildingIconClicked(FName BuildingRowName)
{
	OnBuildMenuSelected.Broadcast(BuildingRowName);
}

void UUW_BuildMenu::SetCurrentCategory(EBuildCategory NewCategory)
{
	if (CurrentCategory == NewCategory)
	{
		return;
	}

	CurrentCategory = NewCategory;
	RefreshBuildingList();
}

void UUW_BuildMenu::RefreshBuildingList()
{
	if (!IsValid(BuildingListWrapBox))
	{
		return;
	}

	BuildingListWrapBox->ClearChildren();

	if (!IsValid(BuildingDataTable) || !BuildingIconWidgetClass)
	{
		return;
	}

	static const FString ContextString(TEXT("BuildMenu"));

	const TArray<FName> RowNames = BuildingDataTable->GetRowNames();
	for (const FName& RowName : RowNames)
	{
		const FBuildingRow* Row = BuildingDataTable->FindRow<FBuildingRow>(RowName, ContextString);
		if (!Row)
		{
			continue;
		}

		if (Row->Category != CurrentCategory)
		{
			continue;
		}

		UUW_BuildingIcon* IconWidget = CreateWidget<UUW_BuildingIcon>(this, BuildingIconWidgetClass);
		if (!IsValid(IconWidget))
		{
			continue;
		}

		UTexture2D* IconTexture = nullptr;
		if (!Row->IconTexture.IsNull())
		{
			IconTexture = Row->IconTexture.LoadSynchronous();
		}

		IconWidget->SetupBuildingIcon(RowName, Row->DisplayName, IconTexture);
		IconWidget->OnBuildingIconClicked.AddDynamic(this, &ThisClass::HandleBuildingIconClicked);

		BuildingListWrapBox->AddChildToWrapBox(IconWidget);
	}
}