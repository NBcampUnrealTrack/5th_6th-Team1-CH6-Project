// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TargetPriority.generated.h"

UENUM(BlueprintType)
enum class ETargetPriorityType : uint8
{
    Ignore  UMETA(DisplayName = "Ignore (0)"),
    High    UMETA(DisplayName = "High (1)"),
    Medium  UMETA(DisplayName = "Medium (2)"),
    Low     UMETA(DisplayName = "Low (3)"),
    Max     UMETA(Hidden)
};