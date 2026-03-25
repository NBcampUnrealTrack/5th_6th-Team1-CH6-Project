#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "EXPAttributeSet.generated.h"

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

UCLASS()
class BULLETANT_API UEXPAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UEXPAttributeSet();

	virtual void PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data) override;
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;

	UFUNCTION()
	virtual void OnRep_CurrentEXP(const FGameplayAttributeData& OldCurrentEXP);
	UFUNCTION()
	virtual void OnRep_MaxEXP(const FGameplayAttributeData& OldMaxEXP);
	UFUNCTION()
	virtual void OnRep_CurrentLevel(const FGameplayAttributeData& OldCurrentLevel);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void LevelUp();
	void UpdateMaxEXP();

public:
	UPROPERTY(BlueprintReadOnly, Category = "EXP", ReplicatedUsing = OnRep_CurrentLevel)
	FGameplayAttributeData CurrentLevel;
	ATTRIBUTE_ACCESSORS(UEXPAttributeSet, CurrentLevel)

	UPROPERTY(BlueprintReadOnly, Category = "EXP", ReplicatedUsing = OnRep_CurrentEXP)
	FGameplayAttributeData CurrentEXP;
	ATTRIBUTE_ACCESSORS(UEXPAttributeSet, CurrentEXP)

	UPROPERTY(BlueprintReadOnly, Category = "EXP", ReplicatedUsing = OnRep_MaxEXP)
	FGameplayAttributeData MaxEXP;
	ATTRIBUTE_ACCESSORS(UEXPAttributeSet, MaxEXP)

	UPROPERTY(BlueprintReadOnly, Category = "Combat|Meta")
	FGameplayAttributeData IncomingEXP;
	ATTRIBUTE_ACCESSORS(UEXPAttributeSet, IncomingEXP)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EXP")
	float MaxLevel = 99.f;

	
};
