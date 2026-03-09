

#include "UI/UW_BuildingIcon.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"


void UUW_BuildingIcon::NativeConstruct()
{
	Super::NativeConstruct();

	if (IsValid(IconButton))
	{
		IconButton->OnClicked.AddDynamic(this, &ThisClass::OnButtonClicked);
	}
}

void UUW_BuildingIcon::SetupBuildingIcon(FName InRowName, const FText& InDisplayName, UTexture2D* InIconTexture)
{
	BuildingRowName = InRowName;

	if (IsValid(BuildingNameText))
	{
		BuildingNameText->SetText(InDisplayName);
	}

	if (IsValid(BuildingIconImage) && InIconTexture)
	{
		BuildingIconImage->SetBrushFromTexture(InIconTexture, true);
	}
}

void UUW_BuildingIcon::OnButtonClicked()
{
	OnBuildingIconClicked.Broadcast(BuildingRowName);
}