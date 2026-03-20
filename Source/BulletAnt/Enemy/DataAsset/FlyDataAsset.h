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
	UPROPERTY(EditAnywhere, Category = "Fly")
	FRotator FlyRotationRate = FRotator(100, 100, 100);

	UPROPERTY(EditDefaultsOnly, Category = "Fly")
	float BrakingDecelerationFlying = 6000.f;	

	UPROPERTY(EditAnywhere, Category = "Fly")
	float AccelerationRate = 2.0f; 

	UPROPERTY(EditAnywhere, Category = "Fly")
	float FlyHeight = 2000.f;

	UPROPERTY(EditAnywhere, Category = "Fly")
	float DestHeightThreshold = 50.f;

	UPROPERTY(EditAnywhere, Category = "Fly")
	float DestHorizontalThreshold = 5.f;

	UPROPERTY(EditAnywhere, Category = "Dive")
	float DiveSpeedMultiplier = 3.0f;

	UPROPERTY(EditAnywhere, Category = "Dive")
	float DiveTotalTime = 3.0f;
};
