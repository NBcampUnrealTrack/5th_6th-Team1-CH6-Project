// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon/Data/WeaponDataAsset.h"
#include "RangedWeaponDataAsset.generated.h"

class ABaseProjectile;

UCLASS()
class BULLETANT_API URangedWeaponDataAsset : public UWeaponDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<ABaseProjectile> ProjectileClass;

	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly, meta = (ClampMin = 0.f))
	float ProjectileRadius;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = 0.f));
	float ProjectileSpeed;

	UPROPERTY(EditDefaultsOnly);
	float BaseDamage;

	UPROPERTY(EditDefaultsOnly, meta = (ClampMin = 1.f));
	float RoundPerMinute;

	UPROPERTY(EditDefaultsOnly, meta = (ClampMin = 1));
	int32 MaxAmmo;

	UPROPERTY(EditDefaultsOnly, meta = (ClampMin = 1.f));
	float Range;

	UPROPERTY(EditDefaultsOnly)
	uint8 bAutoFire : 1 = true;

	UPROPERTY(EditDefaultsOnly, meta = (ClampMin = 1.f));
	int32 FirePerShot = 1;
	
	UPROPERTY(EditDefaultsOnly, meta = (ClampMin = 0.f));
	float SpreadDegree;

	UPROPERTY(EditDefaultsOnly)
	UAnimMontage* ReloadMontage;

	UPROPERTY(EditDefaultsOnly, meta = (EditCondition = "ReloadMontage == nullptr", EditConditionHides))
	float ReloadTime = 5.f;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UGameplayEffect> ReloadEffect;

	UPROPERTY(EditDefaultsOnly)
	uint8 bPlayer : 1 = false;

	UPROPERTY(EditDefaultsOnly, meta = (EditCondition = "bPlayer == true", EditConditionHides))
	float RecoilPitch;

	UPROPERTY(EditDefaultsOnly, meta = (EditCondition = "bPlayer == true", EditConditionHides))
	float RecoilYaw;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (EditCondition = "bPlayer == true", EditConditionHides))
	USoundBase* ReloadSound;
};
