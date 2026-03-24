#include "UI/UW_InteractionItem.h"
#include "Components/TextBlock.h"

void UUW_InteractionItem::SetData(const FInteractionOption& InOption)
{
	if (Text_Key)
	{
		Text_Key->SetText(InOption.Key.GetDisplayName());
	}

	if (Text_Label)
	{
		Text_Label->SetText(InOption.Label);
	}
}