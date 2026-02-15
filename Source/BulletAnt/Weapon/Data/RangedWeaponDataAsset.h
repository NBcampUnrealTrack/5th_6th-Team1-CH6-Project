// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon/Data/WeaponDataAsset.h"
#include "RangedWeaponDataAsset.generated.h"

UCLASS()
class BULLETANT_API URangedWeaponDataAsset : public UWeaponDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly)
	float BaseDamage;

	UPROPERTY(EditDefaultsOnly)
	float RoundPerMinute;

	UPROPERTY(EditDefaultsOnly)
	int32 MaxAmmo;

	UPROPERTY(EditDefaultsOnly)
	float ReloadTime;

	UPROPERTY(EditDefaultsOnly)
	float Range;

	UPROPERTY(EditDefaultsOnly)
	uint8 bAutoFire : 1 = true;

	UPROPERTY(EditDefaultsOnly)
	int32 FirePerShot = 1;
	
	UPROPERTY(EditDefaultsOnly)
	float SpreadDegree;

	UPROPERTY(EditDefaultsOnly)
	float RecoilPitchMax = 0.f;

	UPROPERTY(EditDefaultsOnly)
	float RecoilPitchMin = 0.f;

	UPROPERTY(EditDefaultsOnly)
	float RecoilYawMax = 0.f;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UGameplayEffect> FireCueEffect;
};
