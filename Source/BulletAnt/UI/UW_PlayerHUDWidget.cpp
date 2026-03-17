#include "UI/UW_PlayerHUDWidget.h"
#include "Player/BACharacter.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "UI/UW_WeaponLog.h"
#include "Components/VerticalBox.h"
void UUW_PlayerHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (!OwnerCharacter.IsValid())
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

void UUW_PlayerHUDWidget::AddWeaponLog(UWeaponDataAsset* InData)
{
	if (!WeaponLogClass) return;
	if (WeaponLogBox && WeaponLogBox->GetChildrenCount() > 5)
	{
		WeaponLogBox->GetChildAt(0)->RemoveFromParent();
	}
	
	UUW_WeaponLog* Log = CreateWidget<UUW_WeaponLog>(GetOwningPlayer(), WeaponLogClass);
	if (Log)
	{
		Log->ShowWeaponLog(InData);
		WeaponLogBox->AddChildToVerticalBox(Log);
	}
}

void UUW_PlayerHUDWidget::UpdateHealth(float Current, float Max)
{
	if (!HealthBar || Max <= 0.f)
		return;

	HealthBar->SetPercent(Current / Max);
}

void UUW_PlayerHUDWidget::UpdateAmmo(float Current, float Max)
{
	if (!OwnerCharacter.IsValid()) return;

	if (AmmoText)
	{
		FString AmmoString = FString::Printf(TEXT("%d/%d"), FMath::RoundToInt(Current), FMath::RoundToInt(Max));
		AmmoText->SetText(FText::FromString(AmmoString));
	}
}

