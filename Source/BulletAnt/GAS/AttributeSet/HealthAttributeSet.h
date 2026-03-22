#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "HealthAttributeSet.generated.h"

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

UCLASS()
class BULLETANT_API UHealthAttributeSet : public UAttributeSet
{
	GENERATED_BODY()
	
public:
	UHealthAttributeSet();

	virtual void PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data) override;
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;

	UFUNCTION()
	virtual void OnRep_Health(const FGameplayAttributeData& OldHealth);
	UFUNCTION()
	virtual void OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth);
	UFUNCTION()
	virtual void OnRep_AttackPower(const FGameplayAttributeData& OldAttackPower);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void InitValue(float InHealth = 100.f, float InAttackPower = 0.f);

public:
	UPROPERTY(BlueprintReadOnly, Category = "Health", ReplicatedUsing = OnRep_Health)
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(UHealthAttributeSet, Health)

	UPROPERTY(BlueprintReadOnly, Category = "Health", ReplicatedUsing = OnRep_MaxHealth)
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(UHealthAttributeSet, MaxHealth)
	
	UPROPERTY(BlueprintReadOnly, Category = "Combat|Meta")
	FGameplayAttributeData IncomingDamage;
	ATTRIBUTE_ACCESSORS(UHealthAttributeSet, IncomingDamage)

	UPROPERTY(BlueprintReadOnly, Category = "Combat|Meta")
	FGameplayAttributeData IncreaseMaxHP;
	ATTRIBUTE_ACCESSORS(UHealthAttributeSet, IncreaseMaxHP)

	UPROPERTY(BlueprintReadOnly, Category = "Combat|Meta")
	FGameplayAttributeData IncomingHeal;
	ATTRIBUTE_ACCESSORS(UHealthAttributeSet, IncomingHeal)

	//속성추가
	UPROPERTY(BlueprintReadOnly, Category = "Attack", ReplicatedUsing = OnRep_AttackPower)
	FGameplayAttributeData AttackPower;
	ATTRIBUTE_ACCESSORS(UHealthAttributeSet, AttackPower)

	UPROPERTY(BlueprintReadOnly, Category = "Combat|Meta")
	FGameplayAttributeData IncreaseAttackPower;
	ATTRIBUTE_ACCESSORS(UHealthAttributeSet, IncreaseAttackPower)
};
