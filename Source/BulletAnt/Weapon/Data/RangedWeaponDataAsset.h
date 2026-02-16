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
	float Range;

	UPROPERTY(EditDefaultsOnly)
	uint8 bAutoFire : 1 = true;

	UPROPERTY(EditDefaultsOnly)
	int32 FirePerShot = 1;
	
	UPROPERTY(EditDefaultsOnly)
	float SpreadDegree;

	UPROPERTY(EditDefaultsOnly)
	UAnimMontage* ReloadMontage;

	UPROPERTY(EditDefaultsOnly, meta = (EditCondition = "ReloadMontage == nullptr", EditConditionHides))
	float ReloadTime = 5.f;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UGameplayEffect> FireCueEffect;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UGameplayEffect> ReloadEffect;
};
