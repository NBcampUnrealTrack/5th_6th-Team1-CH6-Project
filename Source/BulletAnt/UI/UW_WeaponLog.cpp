#include "UI/UW_WeaponLog.h"

#include "Components/TextBlock.h"
#include "Weapon/Data/WeaponDataAsset.h"

void UUW_WeaponLog::ShowWeaponLog(UWeaponDataAsset* InData)
{
	if (WeaponName)
	{
		WeaponName->SetText(InData->WeaponName);
		FLinearColor Color = FLinearColor::White;
		switch (InData->Rarity)
		{
		case ERarity::Common :
			Color = FLinearColor::White;
			break;
		case ERarity::Rare :
			Color = FLinearColor::Blue;
			break;
		case ERarity::SuperRare :
			Color = FLinearColor::Green;
			break;
		case ERarity::UltraRare :
			Color = FLinearColor::Red;
			break;
		case ERarity::Legend :
			Color = FLinearColor::Yellow;
			break;
		default:
			break;
		}

		WeaponName->SetColorAndOpacity(FSlateColor(Color));

		GetWorld()->GetTimerManager().SetTimer(
			RemoveTimer,
			this,
			&UUW_WeaponLog::RemoveLog,
			3.f,
			false
		);
	}
}

void UUW_WeaponLog::RemoveLog()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(RemoveTimer);
	}
	RemoveFromParent();
}
