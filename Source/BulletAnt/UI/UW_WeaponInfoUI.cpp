#include "UI/UW_WeaponInfoUI.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Weapon/Data/RangedWeaponDataAsset.h"

void UUW_WeaponInfoUI::NativeConstruct()
{
	Super::NativeConstruct();
}

void UUW_WeaponInfoUI::InitWeaponInfoUI(TSubclassOf<ABaseWeapon> InWeaponClass, URangedWeaponDataAsset* InData)
{
	float DamagePercent = FMath::Clamp(InData->BaseDamage / 100.f, 0.f, 1.f);
	DamageBar->SetPercent(DamagePercent);

	float AccuracyPercent = FMath::Clamp(1 - (InData->SpreadDegree / 100.f), 0.f, 1.f);
	AccuracyBar->SetPercent(AccuracyPercent);
	
	float ReloadPercent = FMath::Clamp(InData->MaxAmmo / 100.f, 0.f, 1.f);
	ReloadBar->SetPercent(ReloadPercent);

	float RPMPercent = FMath::Clamp(InData->RoundPerMinute / 1000.f, 0.f, 1.f);
	RPMBar->SetPercent(RPMPercent);

	WeaponName->SetText(InData->WeaponName);

	WeaponClass = InWeaponClass;
}
