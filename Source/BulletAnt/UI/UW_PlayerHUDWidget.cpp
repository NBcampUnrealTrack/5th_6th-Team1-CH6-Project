#include "UI/UW_PlayerHUDWidget.h"
#include "Player/BACharacter.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "UI/UW_WeaponLog.h"
#include "Components/VerticalBox.h"
#include "UI/UW_OreCount.h"
#include "Framework/BAGameState.h"

void UUW_PlayerHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UUW_PlayerHUDWidget::InitPlayerHUD()
{
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

	if (IsValid(OreCountUI) == true)
	{
		ABAGameState* GS = GetWorld()->GetGameState<ABAGameState>();
		if (IsValid(GS) == true)
		{
			FOnOreChanged::FDelegate Delegate;
			Delegate.BindDynamic(OreCountUI, &UUW_OreCount::SetOreCount);
			GS->BindOnOreChanged(Delegate);

			const auto& OreInventory = GS->GetOreInventory();
			for (const auto& OrePair : OreInventory)
			{
				OreCountUI->SetOreCount(OrePair.Key, OrePair.Value);
			}
		}
	}
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

void UUW_PlayerHUDWidget::ShowAmmoText()
{
	if (AmmoText)
	{
		AmmoText->SetVisibility(ESlateVisibility::Visible);
	}
}

void UUW_PlayerHUDWidget::HideAmmoText()
{
	if (AmmoText)
	{
		AmmoText->SetVisibility(ESlateVisibility::Hidden);
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

