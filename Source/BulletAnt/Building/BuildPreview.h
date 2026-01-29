// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Building/BaseBuilding.h"
#include "Building/BuildingRow.h"
#include "BuildPreview.generated.h"

class UMaterialInterface;
class UMaterialInstanceDynamic;

UCLASS()
class BULLETANT_API ABuildPreview : public ABaseBuilding
{
	GENERATED_BODY()
	
public:
	ABuildPreview();

	void InitWithData(const FBuildingRow& Row);
	void UpdateTransform(const FVector& Location, const FRotator& Rotation);

	void SetCanPlace(bool bInCanPlace);
	bool CanPlace() const { return bCanPlace; }

private:
	UPROPERTY(EditDefaultsOnly)
	UMaterialInterface* PreviewBaseMaterial;

	UPROPERTY()
	UMaterialInstanceDynamic* MID;

	bool bCanPlace = false;
};
