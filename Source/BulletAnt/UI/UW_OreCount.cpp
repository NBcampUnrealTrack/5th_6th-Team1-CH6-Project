#include "UI/UW_OreCount.h"
#include "Components/TextBlock.h"
#include "Mining/VoxelData.h"

void UUW_OreCount::SetOreCount(EVoxelType OreType, int32 OreCount)
{
	// 추후 수정
	UTextBlock* TargetText = nullptr;
	switch (OreType)
	{
		case EVoxelType::Gold:
			TargetText = TextGold;
			break;
		case EVoxelType::Mineral:
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
