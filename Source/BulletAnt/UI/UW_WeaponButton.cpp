#include "UI/UW_WeaponButton.h"

#include "Components/Button.h"
#include "Weapon/BaseWeapon.h"
#include "Components/TextBlock.h"

void UUW_WeaponButton::SetWeaponName(const FText& InName)
{
	if (WeaponName)
	{
		WeaponName->SetText(InName);
	}
}

void UUW_WeaponButton::NativeConstruct()
{
	Super::NativeConstruct();

	if (Button)
	{
		Button->OnClicked.AddDynamic(this, &UUW_WeaponButton::HandleWeaponButtonClick);
	}
}

void UUW_WeaponButton::HandleWeaponButtonClick()
{
	OnClickWeapon.Broadcast(WeaponClass);
}
