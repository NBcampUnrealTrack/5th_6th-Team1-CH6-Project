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
};
