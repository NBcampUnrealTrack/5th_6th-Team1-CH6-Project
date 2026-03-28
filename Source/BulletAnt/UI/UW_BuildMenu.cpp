

#include "UI/UW_BuildMenu.h"
#include "UI/UW_BuildingIcon.h"
#include "Components/Button.h"
#include "Components/VerticalBox.h"
#include "Engine/DataTable.h"
#include "Engine/Texture2D.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Engine/DataTable.h"

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

	ClearSelectedInfo();
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
	SelectedBuildingRowName = BuildingRowName;
	UpdateSelectedInfo(BuildingRowName);
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
	if (!IsValid(BuildingListVerticalBox))
	{
		return;
	}

	BuildingListVerticalBox->ClearChildren();
	ClearSelectedInfo();

	if (!IsValid(BuildingDataTable) || !BuildingIconWidgetClass)
	{
		return;
	}

	static const FString ContextString(TEXT("BuildMenu"));

	const TArray<FName> RowNames = BuildingDataTable->GetRowNames();
	FName FirstRowInCategory = NAME_None;
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

		if (FirstRowInCategory.IsNone())
		{
			FirstRowInCategory = RowName;
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

		BuildingListVerticalBox->AddChildToVerticalBox(IconWidget);
	}

	if (!FirstRowInCategory.IsNone())
	{
		SelectedBuildingRowName = FirstRowInCategory;
		UpdateSelectedInfo(FirstRowInCategory);
	}
}

void UUW_BuildMenu::UpdateSelectedInfo(FName BuildingRowName)
{
	if (!IsValid(BuildingDataTable))
	{
		ClearSelectedInfo();
		return;
	}

	static const FString ContextString(TEXT("BuildMenu_SelectedInfo"));

	const FBuildingRow* Row = BuildingDataTable->FindRow<FBuildingRow>(BuildingRowName, ContextString);
	if (!Row)
	{
		ClearSelectedInfo();
		return;
	}

	if (IsValid(SelectedNameText))
	{
		SelectedNameText->SetText(Row->DisplayName);
	}

	if (IsValid(SelectedIconImage))
	{
		UTexture2D* IconTexture = nullptr;
		if (!Row->IconTexture.IsNull())
		{
			IconTexture = Row->IconTexture.LoadSynchronous();
		}

		SelectedIconImage->SetBrushFromTexture(IconTexture, true);
	}

	if (IsValid(SelectedCostText))
	{
		SelectedCostText->SetText(MakeBuildCostText(Row->BuildCost));
	}

	if (IsValid(SelectedInfoText))
	{
		const FString InfoString = FString::Printf(TEXT("Health : %.0f"), Row->Health);

		SelectedInfoText->SetText(FText::FromString(InfoString));
	}
}

void UUW_BuildMenu::ClearSelectedInfo()
{
	if (IsValid(SelectedNameText))
	{
		SelectedNameText->SetText(FText::GetEmpty());
	}

	if (IsValid(SelectedIconImage))
	{
		SelectedIconImage->SetBrush(FSlateBrush());
	}

	if (IsValid(SelectedCostText))
	{
		SelectedCostText->SetText(FText::GetEmpty());
	}

	if (IsValid(SelectedInfoText))
	{
		SelectedInfoText->SetText(FText::GetEmpty());
	}
}

FText UUW_BuildMenu::MakeBuildCostText(const TMap<EOreType, int32>& BuildCost) const
{
	if (BuildCost.IsEmpty())
	{
		return FText::FromString(TEXT("Cost : None"));
	}

	FString Result = TEXT("-- Cost --\n");

	for (const TPair<EOreType, int32>& Pair : BuildCost)
	{
		FString OreName;

		switch (Pair.Key)
		{
		case EOreType::Gold:
			OreName = TEXT("Gold");
			break;
		case EOreType::Mineral:
			OreName = TEXT("Mineral");
			break;
		default:
			OreName = TEXT("Unknown");
			break;
		}

		Result += FString::Printf(TEXT("%s : %d\n"), *OreName, Pair.Value);
	}

	return FText::FromString(Result);
}
