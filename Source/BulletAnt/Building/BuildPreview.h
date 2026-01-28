// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Building/BuildingRow.h"
#include "BuildPreview.generated.h"

class UStaticMeshComponent;
class UMaterialInterface;
class UMaterialInstanceDynamic;

UCLASS()
class BULLETANT_API ABuildPreview : public AActor
{
	GENERATED_BODY()
	
public:
	ABuildPreview();

	void InitWithData(const FBuildingRow& Row);
	void UpdateTransform(const FVector& Location, const FRotator& Rotation);
	void SetCanPlace(bool bInCanPlace);

	bool CanPlace() const { return bCanPlace; }
	FVector GetPlacementBoxExtent() const { return PlacementBoxExtent; }

private:
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* MeshComp;

	UPROPERTY(EditDefaultsOnly)
	UMaterialInterface* PreviewBaseMaterial;

	UPROPERTY()
	UMaterialInstanceDynamic* MID;

	bool bCanPlace = false;
	FVector PlacementBoxExtent = FVector::ZeroVector;
};
