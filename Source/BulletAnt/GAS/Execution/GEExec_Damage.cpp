#include "GAS/Execution/GEExec_Damage.h"
#include "GAS/AttributeSet/HealthAttributeSet.h"

UGEExec_Damage::UGEExec_Damage()
{
}

void UGEExec_Damage::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();

	float Damage = Spec.GetSetByCallerMagnitude(TAG_Data_Combat_Damage, false);
	Damage = FMath::Max(Damage, 0.f);

	OutExecutionOutput.AddOutputModifier(
		FGameplayModifierEvaluatedData(
			UHealthAttributeSet::GetIncomingDamageAttribute(),
			EGameplayModOp::Additive,
			Damage
		)
	);
}
