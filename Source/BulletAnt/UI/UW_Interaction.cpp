#include "UI/UW_Interaction.h"
#include "UI/UW_InteractionItem.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"

void UUW_Interaction::ClearInteraction()
{
	if (VerticalBox_Options)
	{
		VerticalBox_Options->ClearChildren();
	}
}

void UUW_Interaction::SetInteractionOptions(const TArray<FInteractionOption>& InOptions)
{
	if (!VerticalBox_Options || !InteractionItemClass)
	{
		return;
	}

	VerticalBox_Options->ClearChildren();

	for (const FInteractionOption& Option : InOptions)
	{
		UUW_InteractionItem* Item = CreateWidget<UUW_InteractionItem>(GetWorld(), InteractionItemClass);
		if (!Item)
		{
			continue;
		}

		Item->SetData(Option);

		if (UVerticalBoxSlot* VerticalSlot = VerticalBox_Options->AddChildToVerticalBox(Item))
		{
			VerticalSlot->SetPadding(FMargin(0.f, 0.f, 0.f, 5.f));
		}
	}
}