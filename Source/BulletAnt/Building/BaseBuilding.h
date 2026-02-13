// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BaseBuilding.generated.h"

class UBoxComponent;

USTRUCT()
struct FBuildingEdge
{
	GENERATED_BODY()

	UPROPERTY() FVector A = FVector::ZeroVector;
	UPROPERTY() FVector B = FVector::ZeroVector;

	FVector Mid() const { return (A + B) * 0.5f; }

	FVector2D Dir2D() const
	{
		FVector2D D = FVector2D((B - A).X , (B - A).Y);
		return D.IsNearlyZero() ? FVector2D(1, 0) : D.GetSafeNormal();
	}

	float Length2D() const
	{
		FVector D = (B - A);
		D.Z = 0.f;
		return D.Size();
	}
};

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

	void GetEdgesWorld(TArray<FBuildingEdge>& OutEdges) const;
	void GetEdgesWorldWithTransform(const FTransform& T, TArray<FBuildingEdge>& OutEdges) const;
	void DrawEdgesDebug(bool bPersistentLines, float LifeTime) const;

protected:
	virtual void GetEdgesLocal(TArray<FBuildingEdge>& OutEdges) const;

private:
	UFUNCTION()
	void OnRep_BuildingBoxExtent();

public:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UStaticMeshComponent> StaticMeshComp;

	UPROPERTY(VisibleAnywhere, Category = "Build")
	TObjectPtr<UBoxComponent> BuildingBounds;

protected:
	UPROPERTY(ReplicatedUsing = OnRep_BuildingBoxExtent, VisibleAnywhere, Category = "Build")
	FVector BuildingBoxExtent;
};
