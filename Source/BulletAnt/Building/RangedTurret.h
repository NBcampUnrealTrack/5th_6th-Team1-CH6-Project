// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Building/BaseTurret.h"
#include "Common/FireStartInterface.h"
#include "Common/DataAssetInterface.h"
#include "RangedTurret.generated.h"

class URangedTurretDataAsset;
class URangedWeaponDataAsset;

UCLASS()
class BULLETANT_API ARangedTurret : public ABaseTurret
								  , public IFireStartInterface
								  , public IDataAssetInterface
{
	GENERATED_BODY()

public:
	ARangedTurret();

protected:
	virtual void BeginPlay() override;

protected:
	virtual FVector GetFireStartLocation_Implementation() const override;
	virtual FVector GetFireDirection_Implementation() const override;
	virtual UDataAsset* GetDataAsset() const override;

	virtual void OnDeath() override;
	virtual float GetAttackInterval() const override;
	virtual void ExecuteAttack() override;

	void CollectMuzzleSockets();

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Turret|Data")
	TObjectPtr<URangedTurretDataAsset> RangedTurretData;

	UPROPERTY(Transient)
	TArray<FName> MuzzleSocketNames;

	UPROPERTY(Transient)
	int32 CurrentMuzzleIndex = 0;

	UPROPERTY(Transient)
	int32 NextMuzzleIndex = 0;
};
