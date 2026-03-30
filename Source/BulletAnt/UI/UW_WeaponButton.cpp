#include "UI/UW_WeaponButton.h"

#include "Components/Button.h"
#include "Weapon/BaseWeapon.h"
#include "Components/TextBlock.h"
#include "Weapon/Data/WeaponDataAsset.h"

void UUW_WeaponButton::SetWeaponName(const FText& InName)
{
	if (WeaponName)
	{
		WeaponName->SetText(InName);
		if (WeaponClass)
		{
			ABaseWeapon* Weapon = Cast<ABaseWeapon>(WeaponClass->GetDefaultObject());
			UWeaponDataAsset* Data = Weapon->GetWeaponData();
			WeaponName->SetText(Data->WeaponName);
			FLinearColor Color = FLinearColor::White;
			switch (Data->Rarity)
			{
			case ERarity::Common:
				Color = FLinearColor::White;
				break;
			case ERarity::Rare:
				Color = FLinearColor::Green;
				break;
			case ERarity::SuperRare:
				Color = FLinearColor::Blue;
				break;
			case ERarity::UltraRare:
				Color = FLinearColor::Red;
				break;
			case ERarity::Legend:
				Color = FLinearColor::Yellow;
				break;
			default:
				break;
			}

			WeaponName->SetColorAndOpacity(FSlateColor(Color));
		}
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
