// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BuildPreview.generated.h"

class UBuildingData;
class UStaticMeshComponent;
class UMaterialInterface;
class UMaterialInstanceDynamic;

UCLASS()
class BULLETANT_API ABuildPreview : public AActor
{
	GENERATED_BODY()
	
public:
	ABuildPreview();

	void InitWithData(UBuildingData* InData);
	void UpdateTransform(const FVector& Location, const FRotator& Rotation);
	void SetCanPlace(bool bInCanPlace);

	bool CanPlace() const { return bCanPlace; }
	float GetPlacementRadius() const;

private:
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* MeshComp;

	UPROPERTY(EditDefaultsOnly)
	UMaterialInterface* PreviewBaseMaterial;

	UPROPERTY()
	UMaterialInstanceDynamic* MID;

	UPROPERTY()
	UBuildingData* Data;

	bool bCanPlace = false;
};
