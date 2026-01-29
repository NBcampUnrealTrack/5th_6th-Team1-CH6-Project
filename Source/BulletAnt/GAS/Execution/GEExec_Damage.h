// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectExecutionCalculation.h"
#include "GAS/AttributeSet/HealthAttributeSet.h"
#include "GEExec_Damage.generated.h"

UCLASS()
class BULLETANT_API UGEExec_Damage : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()
	
public:
	UGEExec_Damage();

	virtual void Execute_Implementation(
		const FGameplayEffectCustomExecutionParameters& ExecutionParams,
		FGameplayEffectCustomExecutionOutput& OutExecutionOutput
	) const override;

};
