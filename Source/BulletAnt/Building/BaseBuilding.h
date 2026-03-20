// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AbilitySystemInterface.h"
#include "Common/BAItemInterface.h"
#include "BaseBuilding.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnDestroyed);

class UPrimitiveComponent;
class UAbilitySystemComponent;
class UHealthAttributeSet;
class UGeometryCollection;
class UGeometryCollectionComponent;
class UBuildManagerComponent;

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
								  , public IBAItemInterface
{
	GENERATED_BODY()
	
public:	
	ABaseBuilding();

	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

protected:
	virtual void BeginPlay() override;

public:
	virtual void Use_Implementation(AActor* User) override;

	UFUNCTION()
	void RequestDemolish(AActor* User);

	virtual void OnDeath();

	void GetEdgesWorld(TArray<FBuildingEdge>& OutEdges) const;
	void GetEdgesWorldWithTransform(const FTransform& T, TArray<FBuildingEdge>& OutEdges) const;
	void DrawEdgesDebug(bool bPersistentLines, float LifeTime) const;

	void RebuildCachedLocalEdges();
	void GetPlacementPrimitives(TArray<UPrimitiveComponent*>& OutPrims) const;

	virtual void SetPreviewMode(bool bInPreview);
	virtual void SetCanPlace(bool bInCanPlace);
	bool IsPreviewMode() const { return bPreviewMode; }

	void GetSupportVolumes(TArray<UPrimitiveComponent*>& OutVolumes) const;
	bool ComputeSupportCoverage(const FTransform& WorldT, float& OutCoverage, TSet<TWeakObjectPtr<ABaseBuilding>>& OutSupportBuildings) const;

	void Server_RegisterSupports(const TSet<TWeakObjectPtr<ABaseBuilding>>& Supporters);
	void Server_UnregisterFromSupports();
	void Server_ReevaluateSupportAndMaybeDie();

protected:
	virtual void GetEdgesLocal(TArray<FBuildingEdge>& OutEdges) const;

	UFUNCTION()
	virtual void OnRep_Dead();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayDestruction(const FVector& ImpulseOrigin);

public:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> EdgesRoot;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> PlacementRoot;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> SupportRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UStaticMeshComponent> StaticMeshComp;

	UPROPERTY(EditDefaultsOnly, Category = "Build|Support")
	float MinSupportCoverage = 0.5f;

	UPROPERTY(EditDefaultsOnly, Category = "Build|Support")
	float SupportSampleSpacing = 50.f;

	UPROPERTY(EditDefaultsOnly, Category = "Build|Support")
	float SupportTraceUp = 50.f;

	UPROPERTY(EditDefaultsOnly, Category = "Build|Support")
	float SupportTraceDown = 50.f;

protected:
	UPROPERTY(VisibleAnywhere, Category = "Build|Snap")
	TArray<FBuildingEdge> CachedLocalEdges;

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

	UPROPERTY(EditDefaultsOnly, Category = "Build|Destruction")
	float DebrisLifeSeconds = 5.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Build|Preview")
	bool bPreviewMode = false;

	UPROPERTY(EditDefaultsOnly, Category = "Build|Preview")
	TObjectPtr<UMaterialInterface> PreviewBaseMaterial;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> PreviewMID;

	// 아래 건물들
	UPROPERTY(VisibleAnywhere, Category = "Build|Support")
	TSet<TWeakObjectPtr<ABaseBuilding>> SupportingBuildings;

	// 위 건물들
	UPROPERTY(VisibleAnywhere, Category = "Build|Support")
	TSet<TWeakObjectPtr<ABaseBuilding>> SupportedBuildings;

#pragma region Delegate

	public:
		FOnDestroyed OnDestroyed;

#pragma endregion
};
