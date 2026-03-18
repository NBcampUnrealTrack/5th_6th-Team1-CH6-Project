// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "TurretDataAsset.generated.h"

UENUM(BlueprintType)
enum class ETargetPickPolicy : uint8
{
	Nearest			UMETA(DisplayName = "Nearest"),
};

UCLASS(BlueprintType)
class BULLETANT_API UTurretDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Turret|Targeting", meta = (ClampMin = "0.0"))
	float SearchRadius = 1000.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Turret|Targeting", meta = (ClampMin = "0.01"))
	float TargetSearchInterval = 1.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Turret|Targeting")
	TEnumAsByte<ECollisionChannel> EnemyTraceChannel = ECC_GameTraceChannel6;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Turret|Targeting")
	ETargetPickPolicy TargetPickPolicy = ETargetPickPolicy::Nearest;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Turret|Aim", meta = (ClampMin = "0.0"))
	float TurnSpeedDegPerSec = 180.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Turret|Aim")
	float PitchMin = -20.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Turret|Aim")
	float PitchMax = 90.f;
};