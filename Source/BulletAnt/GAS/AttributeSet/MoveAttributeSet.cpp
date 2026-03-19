// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/AttributeSet/MoveAttributeSet.h"
#include "Net/UnrealNetwork.h"

UMoveAttributeSet::UMoveAttributeSet()
{
	InitMoveSpeed(600.f);
	InitMoveSpeedMultiplier(1.f);
}

void UMoveAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	if (Attribute == GetMoveSpeedAttribute())
	{
		NewValue = FMath::Max(0.f, NewValue);
	}
	else if (Attribute == GetMoveSpeedMultiplierAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.05f, 10.f);
	}
}

void UMoveAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UMoveAttributeSet, MoveSpeed, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMoveAttributeSet, MoveSpeedMultiplier, COND_None, REPNOTIFY_Always);
}

void UMoveAttributeSet::OnRep_MoveSpeed(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMoveAttributeSet, MoveSpeed, OldValue);
}

void UMoveAttributeSet::OnRep_MoveSpeedMultiplier(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMoveAttributeSet, MoveSpeedMultiplier, OldValue);
}
