
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "Building/BuildingRow.h"
#include "BuildManagerComponent.generated.h"

class ABuildPreview;
struct FInputActionValue;
struct FBuildingEdge;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BULLETANT_API UBuildManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UBuildManagerComponent();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	UFUNCTION(BlueprintCallable)
	void EnterBuildMode();

	UFUNCTION(BlueprintCallable)
	void ExitBuildMode();

	UFUNCTION(BlueprintCallable)
	void TryPlace();

	UFUNCTION(BlueprintCallable)
	void RotatePreviewByWheel(const FInputActionValue& Value);

	UFUNCTION(BlueprintCallable)
	void ToggleSnapMode();

	bool IsBuildMode() const { return bBuildMode; }

private:
	bool ComputePreviewPlacement(FVector& OutLocation, FRotator& OutRotation, bool& bOutHasValidSurface);

	bool TrySnapPreview(FVector& InOutLocation, FRotator& InOutRotation);

	UFUNCTION(Server, Reliable)
	void Server_TryPlace(FName BuildingRow, const FVector& Location, const FRotator& Rotation);

	bool CheckCanPlaceAt(const FVector& Location, const FRotator& Rotation, const FVector& InBoxExtent) const;

	void RefreshCachedRef();

	void SetCurrentBuildingRow(FName NewRow);

	void SampleKeyPointsOnEdge(const FBuildingEdge& E, TArray<FVector>& OutPts) const;

	FVector2D ClosestPointOnExtendedLine2D(const FVector2D& Point2D, const FBuildingEdge& TargetEdgeWorld, const FVector2D& TargetDir2D, float HalfRange) const;

	float GetPerpFullSizeForEdge(const ABaseBuilding* Building, const FBuildingEdge& EdgeWorld) const;

private:
	UPROPERTY(EditDefaultsOnly, Category = "Build|Data")
	TObjectPtr<UDataTable> BuildingTable;

	UPROPERTY(EditDefaultsOnly, Category = "Build|Data")
	TSubclassOf<ABuildPreview> PreviewActorClass;

	UPROPERTY(EditDefaultsOnly, Category = "Build|Data")
	FName DefaultBuildingRow = TEXT("TestTurret");

	UPROPERTY()
	FName CurrentBuildingRow;

	UPROPERTY()
	TObjectPtr<ABuildPreview> PreviewActor;

	bool bBuildMode = false;

	UPROPERTY()
	TWeakObjectPtr<AActor> CachedOwner;

	UPROPERTY()
	TWeakObjectPtr<APlayerController> CachedPC;

	const FBuildingRow* CachedBuildingRow = nullptr;

	float CurrentYaw = 0.f;

	UPROPERTY(EditDefaultsOnly, Category = "Build|Rotate")
	float WheelYawStep = 15.f;

	bool bSnapMode = false;

	UPROPERTY(EditDefaultsOnly, Category = "Build|Snap")
	float SnapSearchRadius = 2000.f;

	UPROPERTY(EditDefaultsOnly, Category = "Build|Snap")
	float SnapMaxDistance = 40.f;

	// 면 스냅 cos(각도). 0.7071 ≈ 45도
	UPROPERTY(EditDefaultsOnly, Category = "Build|Snap")
	float EdgeParallelCosThreshold = 0.7071f;

	UPROPERTY(EditDefaultsOnly, Category = "Build|Snap")
	float KeyPointSnapMaxDistance = 100.f;

	UPROPERTY(EditDefaultsOnly, Category = "Build|Placement")
	float MaxBuildDistance = 5000.f;

	UPROPERTY(EditDefaultsOnly, Category = "Build|Placement")
	float AllowedPenetrationDistance = 1.f;

	UPROPERTY(EditDefaultsOnly, Category = "Build|Placement")
	FName GroundActorTag = TEXT("Ground");
};
