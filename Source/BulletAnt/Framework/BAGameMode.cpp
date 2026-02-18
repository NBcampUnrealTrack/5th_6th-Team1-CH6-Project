#include "Framework/BAGameMode.h"
#include "Mining/VoxelData.h"
#include "Framework/BAGameState.h"
#include "Player/BAPlayerController.h"

ABAGameMode::ABAGameMode()
{
	GameStateClass = ABAGameState::StaticClass();
}

void ABAGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	ABAPlayerController* BAPlayerController = Cast<ABAPlayerController>(NewPlayer);
	if (IsValid(BAPlayerController))
	{
		if (ABAGameState* BAGameState = GetGameState<ABAGameState>())
		{
			BAGameState->AddPlayerController(BAPlayerController);
		}
	}
}

void ABAGameMode::Logout(AController* Exiting)
{
	ABAPlayerController* BAPlayerController = Cast<ABAPlayerController>(Exiting);
	if (IsValid(BAPlayerController))
	{
		if (ABAGameState* BAGameState = GetGameState<ABAGameState>())
		{
			BAGameState->RemovePlayerController(BAPlayerController);
		}
	}

	Super::Logout(Exiting);
}

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
