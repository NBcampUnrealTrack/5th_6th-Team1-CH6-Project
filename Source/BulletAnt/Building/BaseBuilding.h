// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AbilitySystemInterface.h"
#include "BaseBuilding.generated.h"

class UBoxComponent;
class UAbilitySystemComponent;
class UHealthAttributeSet;
class UGeometryCollection;
class UGeometryCollectionComponent;

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
								  , public IAbilitySystemInterface
{
	GENERATED_BODY()
	
public:	
	ABaseBuilding();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

protected:
	virtual void BeginPlay() override;

public:
	virtual void OnDeath();

	void ApplyBuildingBounds(const FVector& InBoxExtent);
	FVector GetBuildingBoxExtent() const { return BuildingBoxExtent; }
	void SetBuildingBoxExtent(const FVector& InBoxExtent);

	void GetEdgesWorld(TArray<FBuildingEdge>& OutEdges) const;
	void GetEdgesWorldWithTransform(const FTransform& T, TArray<FBuildingEdge>& OutEdges) const;
	void DrawEdgesDebug(bool bPersistentLines, float LifeTime) const;

protected:
	virtual void GetEdgesLocal(TArray<FBuildingEdge>& OutEdges) const;

	UFUNCTION()
	virtual void OnRep_Dead();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayDestruction(const FVector& ImpulseOrigin);

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

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UAbilitySystemComponent> ASC;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UHealthAttributeSet> HealthSet;

	UPROPERTY(ReplicatedUsing = OnRep_Dead)
	bool bDead = false;

	UPROPERTY(EditDefaultsOnly, Category = "Build|Destruction")
	TObjectPtr<UGeometryCollection> DestructionCollection;

	UPROPERTY(VisibleAnywhere, Category = "Build|Destruction")
	TObjectPtr<UGeometryCollectionComponent> DestructionComp;
};
