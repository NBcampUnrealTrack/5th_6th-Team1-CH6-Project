// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "BuildingData.generated.h"

class ABaseBuilding;

UCLASS(BlueprintType)
class BULLETANT_API UBuildingData : public UDataAsset
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere)
	TSubclassOf<ABaseBuilding> BuildingClass;

	UPROPERTY(EditAnywhere)
	UStaticMesh* PreviewMesh = nullptr;

	UPROPERTY(EditAnywhere)
	float PlacementRadius = 80.f;

	UPROPERTY(EditAnywhere)
	float MaxHP = 100.f;
};
