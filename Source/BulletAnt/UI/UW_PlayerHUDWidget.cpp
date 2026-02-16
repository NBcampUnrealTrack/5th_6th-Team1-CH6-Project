#include "UI/UW_PlayerHUDWidget.h"
#include "Player/BACharacter.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void UUW_PlayerHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (!OwnerCharacter)
		return;

	OwnerCharacter->OnHealthChanged.AddDynamic(
		this,
		&UUW_PlayerHUDWidget::UpdateHealth
	);

	OwnerCharacter->OnAmmoChanged.AddDynamic(
		this,
		&UUW_PlayerHUDWidget::UpdateAmmo
	);
}

void UUW_PlayerHUDWidget::UpdateHealth(float Current, float Max)
{
	if (!HealthBar || Max <= 0.f)
		return;

	HealthBar->SetPercent(Current / Max);
}

void UUW_PlayerHUDWidget::UpdateAmmo(float Current, float Max)
{
	if (!OwnerCharacter) return;

	if (AmmoText)
	{
		FString AmmoString = FString::Printf(TEXT("%d/%d"), FMath::RoundToInt(Current), FMath::RoundToInt(Max));
		AmmoText->SetText(FText::FromString(AmmoString));
	}
}

