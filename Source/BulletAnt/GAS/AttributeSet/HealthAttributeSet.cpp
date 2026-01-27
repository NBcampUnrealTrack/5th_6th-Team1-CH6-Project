#include "GAS/AttributeSet/HealthAttributeSet.h"
#include "GameplayEffectExtension.h"

UHealthAttributeSet::UHealthAttributeSet()
{
	InitHealth(100.f);
	InitMaxHealth(100.f);
}

void UHealthAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		SetHealth(FMath::Clamp(GetHealth(), 0.f, GetMaxHealth()));
	}
}
