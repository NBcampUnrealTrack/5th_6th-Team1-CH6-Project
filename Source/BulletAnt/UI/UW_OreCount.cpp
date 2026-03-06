#include "UI/UW_OreCount.h"
#include "Components/TextBlock.h"
#include "Mining/VoxelData.h"

void UUW_OreCount::SetOreCount(EOreType OreType, int32 OreCount)
{
	// 추후 수정
	UTextBlock* TargetText = nullptr;
	switch (OreType)
	{
		case EOreType::Gold:
			TargetText = TextGold;
			break;
		case EOreType::Mineral:
			TargetText = TextMineral;
			break;
		default:
			break;
	}

	if (IsValid(TargetText) == true)
	{
		TargetText->SetText(FText::FromString(*FString::FromInt(OreCount)));
	}
}
