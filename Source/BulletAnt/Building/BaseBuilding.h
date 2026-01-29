// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BaseBuilding.generated.h"

class UBoxComponent;

UCLASS()
class BULLETANT_API ABaseBuilding : public AActor
{
	GENERATED_BODY()
	
public:	
	ABaseBuilding();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void ApplyBuildingBounds(const FVector& InBoxExtent);
	FVector GetBuildingBoxExtent() const { return BuildingBoxExtent; }
	void SetBuildingBoxExtent(const FVector& InBoxExtent);

	void GetSnapPointsWorld(TArray<FVector>& OutPoints) const;

	void DrawSnapPointsDebug(bool bPersistentLines, float LifeTime) const;

private:
	UFUNCTION()
	void OnRep_BuildingBoxExtent();

	void GetSnapPointsLocal(TArray<FVector>& OutPoints) const;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UStaticMeshComponent> StaticMeshComp;

	UPROPERTY(VisibleAnywhere, Category = "Build")
	TObjectPtr<UBoxComponent> BuildingBounds;

protected:
	UPROPERTY(ReplicatedUsing = OnRep_BuildingBoxExtent, VisibleAnywhere, Category = "Build")
	FVector BuildingBoxExtent;
};
