#include "GAS/AttributeSet/HealthAttributeSet.h"
#include "GameplayEffectExtension.h"
#include "Net/UnrealNetwork.h"
#include "Common/OnDeathInterface.h"
#include "AbilitySystemComponent.h"
#include "GAS/BAGameplayTags.h"
#include "AbilitySystemBlueprintLibrary.h"


UHealthAttributeSet::UHealthAttributeSet()
{
	InitHealth(100.f);
	InitMaxHealth(100.f);
}

void UHealthAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UHealthAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UHealthAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
}

void UHealthAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	if (Data.EvaluatedData.Attribute == GetIncomingDamageAttribute())
	{
		const float LocalIncomingDamage = GetIncomingDamage();

		if (LocalIncomingDamage > 0.f)
		{
			const float NewHealth = GetHealth() - LocalIncomingDamage;
			SetHealth(FMath::Clamp(NewHealth, 0.f, GetMaxHealth()));

			FGameplayEventData Payload;
			Payload.ContextHandle = Data.EffectSpec.GetContext();
			AActor* TargetActor = Data.Target.GetAvatarActor();

			FGameplayCueParameters Params;
			Params.EffectContext = Data.EffectSpec.GetContext();
			Data.Target.ExecuteGameplayCue(TAG_GameplayCue_Combat_Damaged,Params);

			UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
				TargetActor,
				TAG_Event_Combat_Damaged,
				Payload
			);

			if (GetHealth() == 0.f)
			{				
				Payload.EventMagnitude = 5.f;
				
				UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
					TargetActor,
					TAG_Event_Combat_Dead,
					Payload
				);
			}
		}

		SetIncomingDamage(0.f);
	}

	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		SetHealth(FMath::Clamp(GetHealth(), 0.f, GetMaxHealth()));
	}

	if (Data.EvaluatedData.Attribute == GetIncomingHealAttribute())
	{
		const float LocalIncomingHeal = GetIncomingHeal();

		if (LocalIncomingHeal > 0.f)
		{
			const float NewHealth = GetHealth() + LocalIncomingHeal;
			SetHealth(FMath::Clamp(NewHealth, 0.f, GetMaxHealth()));
		}

		SetIncomingHeal(0.f);
	}
}

void UHealthAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeBaseChange(Attribute, NewValue);

	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxHealth());
	}
}

void UHealthAttributeSet::OnRep_Health(const FGameplayAttributeData& OldHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UHealthAttributeSet, Health, OldHealth);
}

void UHealthAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UHealthAttributeSet, MaxHealth, OldMaxHealth);
}


