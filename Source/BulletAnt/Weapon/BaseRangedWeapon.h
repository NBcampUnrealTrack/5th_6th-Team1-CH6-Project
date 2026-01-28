// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon/BaseWeapon.h"
#include "Common/FireStartInterface.h"
#include "BaseRangedWeapon.generated.h"

/**
 * 
 */
UCLASS()
class BULLETANT_API ABaseRangedWeapon : public ABaseWeapon, public IFireStartInterface
{
	GENERATED_BODY()

public:
	virtual FVector GetFireStartLocation() const override;
	virtual FVector GetFireDirection() const override;

	FORCEINLINE FName GetMuzzleSocketName() const { return MuzzleSocketName; }

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Data")
	FName MuzzleSocketName;


};
