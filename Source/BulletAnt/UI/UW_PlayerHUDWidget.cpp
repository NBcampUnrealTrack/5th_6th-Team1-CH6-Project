#include "UI/UW_PlayerHUDWidget.h"
#include "Player/BACharacter.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "UI/UW_WeaponLog.h"
#include "Components/VerticalBox.h"
#include "UI/UW_OreCount.h"
#include "Framework/BAGameState.h"
#include "Components/Image.h"

void UUW_PlayerHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	UpdateLevel(1.f, 1.f);
}

void UUW_PlayerHUDWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (HealthBar)
	{
		DisplayedHealthPercent = FMath::FInterpTo(DisplayedHealthPercent, TargetHealthPercent, InDeltaTime, HealthLerpSpeed);
		HealthBar->SetPercent(DisplayedHealthPercent);
	}

	if (EXPBar)
	{
		DisplayedEXPPercent = FMath::FInterpTo(DisplayedEXPPercent, TargetEXPPercent, InDeltaTime, EXPLerpSpeed);
		EXPBar->SetPercent(DisplayedEXPPercent);
	}
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

	OwnerCharacter->OnEXPChanged.AddDynamic(
		this,
		&UUW_PlayerHUDWidget::UpdateEXP
	);

	OwnerCharacter->OnLevelChanged.AddDynamic(
		this,
		&UUW_PlayerHUDWidget::UpdateLevel
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
	if (CurrentAmmoText && TotalAmmoText && CurrentShotImage)
	{
		CurrentAmmoText->SetVisibility(ESlateVisibility::Visible);
		TotalAmmoText->SetVisibility(ESlateVisibility::Visible);
		CurrentShotImage->SetVisibility(ESlateVisibility::Visible);
	}
}

void UUW_PlayerHUDWidget::HideAmmoText()
{
	if (CurrentAmmoText && TotalAmmoText && CurrentShotImage)
	{
		CurrentAmmoText->SetVisibility(ESlateVisibility::Hidden);
		TotalAmmoText->SetVisibility(ESlateVisibility::Hidden);
		CurrentShotImage->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UUW_PlayerHUDWidget::SetAutoImage(bool bIsFullAuto)
{
	if (!SingleShotImage || !FullAutoShotImage) return;

	if (bIsFullAuto)
	{
		CurrentShotImage->SetBrushFromTexture(FullAutoShotImage);
		CurrentShotImage->SetOpacity(1.f);
	}
	else
	{
		CurrentShotImage->SetBrushFromTexture(SingleShotImage);
		CurrentShotImage->SetOpacity(1.f);
	}
}

void UUW_PlayerHUDWidget::UpdateHealth(float Current, float Max)
{
	if (!HealthBar || Max <= 0.f)
		return;

	TargetHealthPercent = Current / Max;
	ActualHealthBar->SetPercent(Current / Max);
}

void UUW_PlayerHUDWidget::UpdateEXP(float Current, float Max)
{
	if (!EXPBar || Max <= 0.f)
	{
		return;
	}

	TargetEXPPercent = Current / Max;
}

void UUW_PlayerHUDWidget::UpdateAmmo(float Current, float Max)
{
	if (!OwnerCharacter.IsValid()) return;

	if (CurrentAmmoText)
	{
		CurrentAmmoText->SetText(FText::AsNumber(Current));
	}

	if (TotalAmmoText)
	{
		TotalAmmoText->SetText(FText::AsNumber(Max));
	}
}

void UUW_PlayerHUDWidget::UpdateLevel(float Current, float OldLevel)
{
	if (!CurrentLevelText) return;

	FString LevelStr = FString::Printf(TEXT("LV %d"), (int32)Current);
	CurrentLevelText->SetText(FText::FromString(LevelStr));
}

