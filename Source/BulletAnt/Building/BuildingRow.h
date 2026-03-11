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
    FName BuildingId;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Display")
    FText DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TSubclassOf<ABaseBuilding> BuildingClass;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Info")
    TMap<EOreType, int32> BuildCost;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Info")
    EBuildCategory Category = EBuildCategory::Turret;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Info")
    int32 Order = 0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Support")
    float MinSupportCoverage = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Support")
    float SupportSampleSpacing = 50.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stat")
    float Health = 100.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Display")
    TSoftObjectPtr<UTexture2D> IconTexture;
};
