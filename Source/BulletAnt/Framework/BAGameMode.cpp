#include "Framework/BAGameMode.h"
#include "Mining/VoxelData.h"
#include "Framework/BAGameState.h"
#include "Player/BAPlayerController.h"

ABAGameMode::ABAGameMode()
{
	GameStateClass = ABAGameState::StaticClass();

    bUseSeamlessTravel = true;
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

void ABAGameMode::HandleSeamlessTravelPlayer(AController*& C)
{
    Super::HandleSeamlessTravelPlayer(C);

    ABAPlayerController* PC = Cast<ABAPlayerController>(C);
    if (IsValid(PC) == false)
        return;

    PC->SetLevelType(ELevelType::Main);
}

void ABAGameMode::MineOre(EOreType OreType, int32 PointCount)
{
    if (OreType == EOreType::None)
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

bool ABAGameMode::TrySpendOre(const TMap<EOreType, int32>& Cost)
{
    if (!HasAuthority())
    {
        return false;
    }

    ABAGameState* GS = GetGameState<ABAGameState>();
    if (!IsValid(GS))
    {
        return false;
    }

    if (Cost.Num() == 0)
    {
        return true;
    }

    // 보유 광물 체크
    for (const TPair<EOreType, int32>& Pair : Cost)
    {
        const EOreType Type = Pair.Key;
        const int32 Need = Pair.Value;

        if (Need <= 0)
        {
            continue;
        }

        const int32 CurrOreCount = GS->GetOreCount(Type);
        if (CurrOreCount < Need)
        {
            return false;
        }
    }

    // 광물 소모
    for (const TPair<EOreType, int32>& Pair : Cost)
    {
        const EOreType Type = Pair.Key;
        const int32 Need = Pair.Value;

        if (Need <= 0)
        {
            continue;
        }

        const int32 CurrOreCount = GS->GetOreCount(Type);
        GS->SetOreCount(Type, CurrOreCount - Need);
    }

    return true;
}
