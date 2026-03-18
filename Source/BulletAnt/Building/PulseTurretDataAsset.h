// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Building/TurretDataAsset.h"
#include "PulseTurretDataAsset.generated.h"

class UGameplayEffect;
class USoundBase;
class UNiagaraSystem;

UCLASS(BlueprintType)
class BULLETANT_API UPulseTurretDataAsset : public UTurretDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Turret|Pulse", meta = (ClampMin = "0.0"))
	float PulseRadius = 1000.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Turret|Pulse", meta = (ClampMin = "0.01"))
	float PulseInterval = 2.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Turret|Pulse", meta = (ClampMin = "0.0"))
	float PulseDamage = 10.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Turret|Pulse")
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Turret|Pulse")
	TSubclassOf<UGameplayEffect> SlowEffectClass;

	UPROPERTY(EditDefaultsOnly, Category = "Pulse|FX")
	TObjectPtr<UNiagaraSystem> PulseNiagara;

	UPROPERTY(EditDefaultsOnly, Category = "Pulse|FX")
	TObjectPtr<USoundBase> PulseSound;

	UPROPERTY(EditDefaultsOnly, Category = "Pulse|FX")
	FVector FXScale = FVector(1.f);
};