// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Mining/VoxelData.h" 
#include "BuildingRow.generated.h"

class ABaseBuilding;

UENUM(BlueprintType)
enum class EBuildCategory : uint8
{
    Turret,
    Building,
    Etc,
};

USTRUCT(BlueprintType)
struct FBuildingRow : public FTableRowBase
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TSubclassOf<ABaseBuilding> BuildingClass;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TMap<EVoxelType, int32> BuildCost;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    EBuildCategory Category = EBuildCategory::Turret;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    int32 Order = 0;
};
