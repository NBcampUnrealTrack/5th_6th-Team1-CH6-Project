#include "UI/UW_RespawnBar.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void UUW_RespawnBar::NativeConstruct()
{

}

void UUW_RespawnBar::UpdateRespawnBar(float CurrentTime, float TotalTime)
{
	float LeftTime = TotalTime - CurrentTime;
	LeftRespawnTime->SetText(FText::FromString(FString::Printf(TEXT("%.1f"), LeftTime)));
	RespawnBar->SetPercent(CurrentTime / TotalTime);
}
