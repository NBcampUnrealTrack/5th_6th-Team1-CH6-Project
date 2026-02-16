#include "GAS/AttributeSet/AmmoAttributeSet.h"
#include "GameplayEffectExtension.h"
#include "Net/UnrealNetwork.h"

UAmmoAttributeSet::UAmmoAttributeSet()
{
	InitCurrentAmmo(30.f);
	InitMaxAmmo(30.f);
}

void UAmmoAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	if (Data.EvaluatedData.Attribute == GetFireAmmoAttribute())
	{
		const float InFireAmmo = GetFireAmmo();

		if (InFireAmmo > 0)
		{
			const float NewAmmo = FMath::Clamp(GetCurrentAmmo() - InFireAmmo, 0.f, GetMaxAmmo());
			SetCurrentAmmo(NewAmmo);
		}
	}

	if (Data.EvaluatedData.Attribute == GetReloadingAmmoAttribute())
	{
		const float NewAmmo = GetReloadingAmmo();
		SetCurrentAmmo(FMath::Clamp(NewAmmo, 0, GetMaxAmmo()));
	}

	SetFireAmmo(0.f);
	SetReloadingAmmo(0.f);
}

void UAmmoAttributeSet::OnRep_CurrentAmmo(const FGameplayAttributeData& OldCurrentAmmo)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAmmoAttributeSet, CurrentAmmo, OldCurrentAmmo);
}

void UAmmoAttributeSet::OnRep_MaxAmmo(const FGameplayAttributeData& OldMaxAmmo)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAmmoAttributeSet, MaxAmmo, OldMaxAmmo);
}

void UAmmoAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UAmmoAttributeSet, CurrentAmmo, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAmmoAttributeSet, MaxAmmo, COND_None, REPNOTIFY_Always);
}
