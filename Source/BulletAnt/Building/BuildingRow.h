// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Mining/VoxelData.h" 
#include "BuildingRow.generated.h"

class ABaseBuilding;

USTRUCT(BlueprintType)
struct FBuildingRow : public FTableRowBase
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TSubclassOf<ABaseBuilding> BuildingClass;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TMap<EVoxelType, int32> BuildCost;
};
