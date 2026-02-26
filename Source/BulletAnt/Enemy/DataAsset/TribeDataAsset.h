// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Enemy/DataAsset/TargetPriority.h"
#include "TribeDataAsset.generated.h"

/**
 * 
 */
UCLASS()
class BULLETANT_API UTribeDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:

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
