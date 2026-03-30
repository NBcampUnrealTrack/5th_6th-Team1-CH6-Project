// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/UW_WaveTimer.h"
#include "Components/TextBlock.h"
#include "Framework/BAGameState.h"
#include "Components/Image.h"
#include "Audio/BABGMManager.h"
#include "Kismet/GameplayStatics.h"

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
    BGMManager = Cast<ABABGMManager>(UGameplayStatics::GetActorOfClass(GetWorld(), ABABGMManager::StaticClass()));
    if (BGMManager)
    {
        BGMManager->StartDayPhase();
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

    if (IsValid(DateBlock))
    {
        DateBlock->SetText(FText::FromString(FString::Printf(TEXT("Day %d"), CachedGameState->GetDate())));
    }

    if (IsValid(HandImage))
    {
        UpdateClockRotation(InitWaveTime, RemainingTime);
    }

    if (InitWaveTime == RemainingTime)
    {
        BGMManager->StartDayPhase();
    }
    else if(RemainingTime <= 100.f)
        BGMManager->StartNightPhase();

}

void UUW_WaveTimer::UpdateClockRotation(const int InitTime, const int CurrentTime)
{
    float Progress = 1.0f - (static_cast<float>(CurrentTime) / InitTime);
    Progress = FMath::Clamp(Progress, 0.0f, 1.0f);

    float NewAngle = Progress * 360.f;

    HandImage->SetRenderTransformAngle(-NewAngle);
}
