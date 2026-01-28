// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
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
    UStaticMesh* PreviewMesh;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FVector PlacementBoxExtent = FVector(100.f, 100.f, 100.f);
};
