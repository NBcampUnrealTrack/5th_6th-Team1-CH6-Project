#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "BAAbilitySystemComponent.generated.h"

UCLASS()
class BULLETANT_API UBAAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effect")
	TSubclassOf<UGameplayEffect> RespawnHealEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effect")
	TSubclassOf<UGameplayEffect> DeadStateEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AnimMontage")
	TObjectPtr<UAnimMontage> HitMontage;

	
};
