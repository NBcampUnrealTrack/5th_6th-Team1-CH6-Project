#include "UI/UW_PlayerHUDWidget.h"
#include "Player/BACharacter.h"
#include "Components/ProgressBar.h"

void UUW_PlayerHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (!OwnerCharacter)
		return;

	OwnerCharacter->OnHealthChanged.AddDynamic(
		this,
		&UUW_PlayerHUDWidget::UpdateHealth
	);
}

void UUW_PlayerHUDWidget::UpdateHealth(float Current, float Max)
{
	if (!HealthBar || Max <= 0.f)
		return;

	HealthBar->SetPercent(Current / Max);
}
