// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Execution/GEExec_Damage.h"
#include "GAS/AttributeSet/HealthAttributeSet.h"

UGEExec_Damage::UGEExec_Damage()
{
}

void UGEExec_Damage::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	
	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();

	static FGameplayTag DamageTag =
		FGameplayTag::RequestGameplayTag(TEXT("Event.Combat.Hit"));

	float Damage = Spec.GetSetByCallerMagnitude(DamageTag, false);
	UE_LOG(LogTemp, Error, TEXT("Execte_Implementation %.2f"),Damage);
	Damage = FMath::Max(Damage, 0.f);

	OutExecutionOutput.AddOutputModifier(
		FGameplayModifierEvaluatedData(
			UHealthAttributeSet::GetIncomingDamageAttribute(),
			EGameplayModOp::Additive,
			Damage
		)
	);
}
