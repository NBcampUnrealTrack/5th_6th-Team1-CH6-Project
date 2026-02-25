// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/DataTable.h"
#include "Enemy/Tribe/TargetPriority.h"
#include "TribeEntry.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct BULLETANT_API FTribeEntry : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category = "Tribe Color")
	FLinearColor TribeColor = FLinearColor(0, 0, 0, 0);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Target Priority")
	ETargetPriorityType Core = ETargetPriorityType::High;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Target Priority")
	ETargetPriorityType Player = ETargetPriorityType::Low;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Target Priority")
	ETargetPriorityType Building = ETargetPriorityType::Medium;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tribe Multiplier")
	float HealthMul = 1.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tribe Multiplier")
	float SpeedMul = 1.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tribe Multiplier")
	float AttackMul = 1.f;
};
