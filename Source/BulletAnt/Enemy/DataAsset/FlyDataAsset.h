// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseEnemyDataAsset.h"
#include "FlyDataAsset.generated.h"

UCLASS()
class BULLETANT_API UFlyDataAsset : public UBaseEnemyDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, Category = "Fly")
	float BrakingDecelerationFlying = 6000.f;	

	UPROPERTY(EditAnywhere, Category = "Fly")
	float AccelerationRate = 2.0f; 

	UPROPERTY(EditAnywhere, Category = "Fly")
	float TurnSpeed = 3.0f;

	UPROPERTY(EditAnywhere, Category = "Dive")
	float DiveTotalTime = 3.0f;
};
