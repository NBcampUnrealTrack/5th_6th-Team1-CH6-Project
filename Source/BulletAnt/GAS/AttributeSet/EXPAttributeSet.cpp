#include "GAS/AttributeSet/EXPAttributeSet.h"

#include "Net/UnrealNetwork.h"
#include "GameplayEffectExtension.h"
#include "AbilitySystemComponent.h"
#include "GAS/BAGameplayTags.h"

UEXPAttributeSet::UEXPAttributeSet()
{
	InitCurrentEXP(0.f);
	InitMaxEXP(100.f);
	InitCurrentLevel(1.f);
}

void UEXPAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	if (Data.EvaluatedData.Attribute == GetIncomingEXPAttribute())
	{
		if (GetCurrentLevel() >= MaxLevel) return;

		const float LocalIncomingEXP = GetIncomingEXP();

		if (LocalIncomingEXP > 0)
		{
			float NewCurrentEXP = GetCurrentEXP() + LocalIncomingEXP;

			int32 Safety = 0;

			while (NewCurrentEXP >= GetMaxEXP() && Safety < 100)
			{
				NewCurrentEXP -= GetMaxEXP();
				LevelUp();

				if (GetCurrentLevel() >= MaxLevel)
				{
					SetCurrentEXP(GetMaxEXP());
					FGameplayCueParameters Params;
					Data.Target.ExecuteGameplayCue(TAG_GameplayCue_Reward_LevelUp, Params);
					break;
				}
				Safety++;
			}

			SetCurrentEXP(FMath::Clamp(NewCurrentEXP, 0.f, GetMaxEXP()));		
		}
	}
	SetIncomingEXP(0.f);
}

void UEXPAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);
}

void UEXPAttributeSet::OnRep_CurrentEXP(const FGameplayAttributeData& OldCurrentEXP)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UEXPAttributeSet, CurrentEXP, OldCurrentEXP);
}

void UEXPAttributeSet::OnRep_MaxEXP(const FGameplayAttributeData& OldMaxEXP)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UEXPAttributeSet, MaxEXP, OldMaxEXP);
}

void UEXPAttributeSet::OnRep_CurrentLevel(const FGameplayAttributeData& OldCurrentLevel)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UEXPAttributeSet, CurrentLevel, OldCurrentLevel);
}

void UEXPAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UEXPAttributeSet, CurrentEXP, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UEXPAttributeSet, MaxEXP, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UEXPAttributeSet, CurrentLevel, COND_None, REPNOTIFY_Always);
}

void UEXPAttributeSet::LevelUp()
{
	SetCurrentLevel(GetCurrentLevel() + 1.f);	

	UpdateMaxEXP();
}

void UEXPAttributeSet::UpdateMaxEXP()
{
	float BaseEXP = 100.f;
	float NewMaxEXP = BaseEXP * FMath::Pow(1.2f, GetCurrentLevel());
	
	SetMaxEXP(NewMaxEXP);
}
