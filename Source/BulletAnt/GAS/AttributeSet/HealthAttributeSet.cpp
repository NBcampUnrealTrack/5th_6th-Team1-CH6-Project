#include "GAS/AttributeSet/HealthAttributeSet.h"
#include "GameplayEffectExtension.h"
#include "Net/UnrealNetwork.h"
#include "Common/OnDeathInterface.h"
#include "AbilitySystemComponent.h"
#include "GAS/BAGameplayTags.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Player/BACharacter.h"
#include "GAS/AttributeSet/EXPAttributeSet.h"
#include "Enemy/BaseEnemy/BaseEnemyCharacter.h"

UHealthAttributeSet::UHealthAttributeSet()
{
	InitHealth(100.f);
	InitMaxHealth(100.f);
	InitAttackPower(0.f);
}

void UHealthAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UHealthAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UHealthAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
}

void UHealthAttributeSet::InitValue(float InHealth, float InAttackPower)
{
	SetMaxHealth(InHealth);
	SetHealth(InHealth);
	SetAttackPower(InAttackPower);
}

void UHealthAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	if (Data.EvaluatedData.Attribute == GetIncomingDamageAttribute())
	{
		UAbilitySystemComponent* TargetASC = Data.Target.AbilityActorInfo->AbilitySystemComponent.Get();
		if (!TargetASC || TargetASC->HasMatchingGameplayTag(TAG_State_Combat_Dead)) return;

		float LocalIncomingDamage = GetIncomingDamage();

		if (LocalIncomingDamage > 0.f)
		{
			UAbilitySystemComponent* InstigatorASC = Data.EffectSpec.GetContext().GetOriginalInstigatorAbilitySystemComponent();
			if (InstigatorASC && InstigatorASC->HasMatchingGameplayTag(TAG_Team_Player) && TargetASC->HasMatchingGameplayTag(TAG_Team_Player))
			{
				LocalIncomingDamage = LocalIncomingDamage * 0.05f;
			}
			const float NewHealth = GetHealth() - LocalIncomingDamage;
			SetHealth(FMath::Clamp(NewHealth, 0.f, GetMaxHealth()));

			FGameplayEventData Payload;
			Payload.ContextHandle = Data.EffectSpec.GetContext();
			AActor* TargetActor = Data.Target.GetAvatarActor();

			FGameplayCueParameters Params;
			Params.EffectContext = Data.EffectSpec.GetContext();
			Params.RawMagnitude = LocalIncomingDamage;
			Data.Target.ExecuteGameplayCue(TAG_GameplayCue_Combat_Damaged, Params);

			UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
				TargetActor,
				TAG_Event_Combat_Damaged,
				Payload
			);

			if (GetHealth() == 0.f)
			{	
				ABACharacter* PlayerCharacter = Cast<ABACharacter>(Data.EffectSpec.GetContext().GetInstigator());
				if (PlayerCharacter)
				{
					if (ABaseEnemyCharacter* Enemy = Cast<ABaseEnemyCharacter>(TargetActor))
					{
						if (Enemy)
						{
							float EXP = Enemy->GetEXP();
							PlayerCharacter->GetEXP(EXP);
						}
					}
				}

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

	if (Data.EvaluatedData.Attribute == GetIncreaseMaxHPAttribute())
	{
		const float LocalIncreaseMaxHealth = GetIncreaseMaxHP();

		SetMaxHealth(GetMaxHealth() + LocalIncreaseMaxHealth);
		SetHealth(GetMaxHealth());

		SetIncreaseMaxHP(0.f);
	}

	if (Data.EvaluatedData.Attribute == GetIncreaseAttackPowerAttribute())
	{
		const float LocalIncreaseAttackPower = GetIncreaseAttackPower();

		SetAttackPower(GetAttackPower() + LocalIncreaseAttackPower);

		SetIncreaseAttackPower(0.f);
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
	Super::PreAttributeChange(Attribute, NewValue);

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

void UHealthAttributeSet::OnRep_AttackPower(const FGameplayAttributeData& OldAttackPower)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UHealthAttributeSet, AttackPower, OldAttackPower);
}


