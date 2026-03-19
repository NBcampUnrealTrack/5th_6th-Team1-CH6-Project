// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Building/TurretDataAsset.h"
#include "RangedTurretDataAsset.generated.h"

class URangedWeaponDataAsset;

UCLASS(BlueprintType)
class BULLETANT_API URangedTurretDataAsset : public UTurretDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Turret|Weapon")
	TObjectPtr<URangedWeaponDataAsset> WeaponData;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Turret|Muzzle")
	FName MuzzleSocketPrefix = TEXT("Muzzle");
};