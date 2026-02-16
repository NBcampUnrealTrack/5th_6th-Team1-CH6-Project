#include "Framework/BAGameMode.h"
#include "Mining/VoxelData.h"
#include "Framework/BAGameState.h"

void ABAGameMode::MineOre(EVoxelType OreType, int32 PointCount)
{
	if (OreType == EVoxelType::BedRock ||
		OreType == EVoxelType::NormalRock ||
		OreType == EVoxelType::None)
		return;

	if (PointCount <= 0)
		return;

	ABAGameState* GS = GetGameState<ABAGameState>();
	if (IsValid(GS) == false)
		return;

	int32 MinGain = PointCount * OreMultiplierMin;
	int32 MaxGain = PointCount * OreMultiplierMax;
	int32 GainedCount = FMath::RandRange(MinGain, MaxGain);

	int32 CurrOreCount = GS->GetOreCount(OreType);
	int32 TotalOreCount = CurrOreCount + GainedCount;
	GS->SetOreCount(OreType, TotalOreCount);
}
