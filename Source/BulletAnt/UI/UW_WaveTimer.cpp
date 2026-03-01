// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/UW_WaveTimer.h"
#include "Components/TextBlock.h"
#include "Framework/BAGameState.h"

void UUW_WaveTimer::NativeConstruct()
{
	Super::NativeConstruct();

    UWorld* World = GetWorld();
    if (!IsValid(World)) return;

    CachedGameState = World->GetGameState<ABAGameState>();
    if (!ensureMsgf(IsValid(CachedGameState), TEXT("UW_WaveTimer NativeConstruct : GameState Error")))
    {
        return;
    }

    CachedGameState->OnWaveTimeChanged.AddUObject(this, &UUW_WaveTimer::UpdateTime);

    UpdateTime();
}

void UUW_WaveTimer::UpdateTime()
{
    if (!IsValid(CachedGameState))
    {
        return;
    }

    int32 InitWaveTime = FMath::Max(0, CachedGameState->GetInitWavePreparationTime());
    int32 RemainingTime = FMath::Max(0, CachedGameState->GetWavePreparationTime());

    if (RemainingTime == 0)
    {
        TimeBlock->SetText(FText::FromString(TEXT("Monster Approaching")));
        TimeBlock->SetColorAndOpacity(EndColor);
    }
    else
    {
        TimeBlock->SetText(FText::FromString(FString::Printf(TEXT("Remaining Time : %d"), RemainingTime)));

        float Alpha = (InitWaveTime == 0) ? 1 : FMath::Clamp(static_cast<float>(RemainingTime) / InitWaveTime, 0.0f, 1.0f);
        SetColor(Alpha);
    }
}

void UUW_WaveTimer::SetColor(float Alpha)
{
    FLinearColor FinalColor;
    if (Alpha > 0.5f)
    {
        Alpha = (Alpha - 0.5f) * 2;
        FinalColor = FLinearColor::LerpUsingHSV(MidColor, StartColor, Alpha);
    }
    else
    {
        Alpha = Alpha * 2;
        FinalColor = FLinearColor::LerpUsingHSV(EndColor, MidColor, Alpha);
    }

    TimeBlock->SetColorAndOpacity(FinalColor);
}
