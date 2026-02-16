// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon/BaseWeapon.h"
#include "Common/FireStartInterface.h"
#include "GameplayTagContainer.h"
#include "BaseRangedWeapon.generated.h"

class URangedWeaponDataAsset;

UCLASS()
class BULLETANT_API ABaseRangedWeapon : public ABaseWeapon
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;

	FORCEINLINE FName GetMuzzleSocketName() const { return MuzzleSocketName; }

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Data")
	FName MuzzleSocketName;

	UPROPERTY()
	URangedWeaponDataAsset* Data;
};
