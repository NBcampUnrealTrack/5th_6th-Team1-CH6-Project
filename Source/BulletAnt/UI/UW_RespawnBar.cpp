#include "UI/UW_RespawnBar.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void UUW_RespawnBar::UpdateRespawnBar(float CurrentTime, float TotalTime)
{
	if (!RespawnBar || !LeftRespawnTime || TotalTime <= 0.f) return;

	float LeftTime = TotalTime - CurrentTime;
	LeftRespawnTime->SetText(FText::FromString(FString::Printf(TEXT("%.1f"), LeftTime)));
	RespawnBar->SetPercent(CurrentTime / TotalTime);
}
