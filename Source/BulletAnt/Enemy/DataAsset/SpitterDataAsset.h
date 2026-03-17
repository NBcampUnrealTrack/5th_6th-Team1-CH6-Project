// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "SpitterDataAsset.generated.h"

/**
 * 
 */
UCLASS()
class BULLETANT_API USpitterDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
    UPROPERTY(EditDefaultsOnly)
    FName AttackOrigin;

    UPROPERTY(EditDefaultsOnly)
    int32 PoisonCapsuleRadius = 40;

    UPROPERTY(EditDefaultsOnly)
    int32 PoisonCapsuleHalfHeight = 250;

    UPROPERTY(EditDefaultsOnly)
    float CheckInterval = 0.1f;
};
