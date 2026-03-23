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
#pragma region Fly

	UPROPERTY(EditDefaultsOnly, Category = "Fly")
	FRotator FlyRotationRate = FRotator(100, 100, 100);

	UPROPERTY(EditDefaultsOnly, Category = "Fly")
	float BrakingDecelerationFlying = 6000.f;	

	UPROPERTY(EditDefaultsOnly, Category = "Fly")
	float AccelerationRate = 2.0f; 

	UPROPERTY(EditDefaultsOnly, Category = "Fly")
	float FlyHeight = 2000.f;

	UPROPERTY(EditDefaultsOnly, Category = "Fly")
	float DestHeightThreshold = 50.f;

	UPROPERTY(EditDefaultsOnly, Category = "Fly")
	float DestHorizontalThreshold = 5.f;

#pragma endregion

#pragma region Dive

	UPROPERTY(EditDefaultsOnly, Category = "Dive")
	float DiveSpeedMultiplier = 3.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Dive")
	float DiveTotalTime = 3.0f;

#pragma endregion

#pragma region Attack

	UPROPERTY(EditDefaultsOnly, Category = "Attack")
	float AttackBoxHalfWidth = 50.f;

	UPROPERTY(EditDefaultsOnly, Category = "Attack")
	float AttackBoxHalfHeight = 100.f;

	UPROPERTY(EditDefaultsOnly, Category = "Attack")
	float AttackBoxHalfDepth = 15.f;

#pragma endregion
};
