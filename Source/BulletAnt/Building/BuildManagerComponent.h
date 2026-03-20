
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "Building/BuildingRow.h"
#include "BuildManagerComponent.generated.h"

class ABaseBuilding;
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

	UFUNCTION()
	void OnBuildMenuSelected(FName NewRow);

	UFUNCTION(BlueprintCallable)
	void TryPlace();

	UFUNCTION(BlueprintCallable)
	void RotatePreviewByWheel(const FInputActionValue& Value);

	UFUNCTION(BlueprintCallable)
	void ToggleSnapMode();

	UFUNCTION(BlueprintCallable)
	void ToggleBuildMenu();

	bool IsBuildMenuOpen() const { return bBuildMenuOpen; }

	bool IsBuildMode() const { return bBuildMode; }

	void OnSelectCat1() { SelectCategory(EBuildCategory::Turret); }
	void OnSelectCat2() { SelectCategory(EBuildCategory::Building); }
	void OnSelectCat3() { SelectCategory(EBuildCategory::Etc); }

	void OnCyclePrev() { CycleInCategory(-1); }
	void OnCycleNext() { CycleInCategory(+1); }


private:
	void SpawnPreview(TSubclassOf<ABaseBuilding> BuildingClass);
	bool ComputePreviewPlacement(FVector& OutLocation, FRotator& OutRotation, bool& bOutHasValidSurface);
	bool TrySnapPreview(FVector& InOutLocation, FRotator& InOutRotation);

	UFUNCTION(Server, Reliable)
	void Server_TryPlace(FName BuildingRow, const FVector& Location, const FRotator& Rotation);

	bool CheckCanPlaceAt() const;

	void RefreshCachedRef();
	void RefreshCategoryCache();
	void SetCurrentBuildingRow(FName NewRow);

	void SampleKeyPointsOnEdge(const FBuildingEdge& E, TArray<FVector>& OutPts) const;

	FVector2D ClosestPointOnExtendedLine2D(const FVector2D& Point2D, const FBuildingEdge& TargetEdgeWorld, const FVector2D& TargetDir2D, float HalfRange) const;

	UFUNCTION()
	void SelectCategory(EBuildCategory NewCategory);

	UFUNCTION()
	void CycleInCategory(int32 Delta);

private:
	UPROPERTY(EditDefaultsOnly, Category = "Build|Data")
	TObjectPtr<UDataTable> BuildingTable;

	UPROPERTY(EditDefaultsOnly, Category = "Build|Data")
	FName DefaultBuildingRow = TEXT("TestTurret");

	EBuildCategory CurrentCategory = EBuildCategory::Turret;
	int32 CurrentIndexInCategory = 0;

	TMap<EBuildCategory, TArray<FName>> CategoryRows;

	UPROPERTY()
	FName CurrentBuildingRow;

	UPROPERTY()
	TObjectPtr<ABaseBuilding> PreviewActor;

	bool bBuildMode = false;
	bool bCanPlace = false;

	UPROPERTY()
	TWeakObjectPtr<AActor> CachedOwner;

	UPROPERTY()
	TWeakObjectPtr<APlayerController> CachedPC;

	const FBuildingRow* CachedBuildingRow = nullptr;

	float CurrentYaw = 0.f;

	UPROPERTY(EditDefaultsOnly, Category = "Build|Rotate")
	float WheelYawStep = 15.f;

	bool bSnapMode = true;

	UPROPERTY(EditDefaultsOnly, Category = "Build|Snap")
	float SnapSearchRadius = 4000.f;

	UPROPERTY(EditDefaultsOnly, Category = "Build|Snap")
	float SnapMaxDistance = 50.f;

	// 면 스냅 cos(각도). 0.7071 ≈ 45도
	UPROPERTY(EditDefaultsOnly, Category = "Build|Snap")
	float EdgeParallelCosThreshold = 0.7071f;

	UPROPERTY(EditDefaultsOnly, Category = "Build|Snap")
	float KeyPointSnapMaxDistance = 100.f;

	UPROPERTY(EditDefaultsOnly, Category = "Build|Placement")
	float MaxBuildDistance = 2000.f;

	UPROPERTY(EditDefaultsOnly, Category = "Build|Placement")
	float AllowedPenetrationDistance = 1.f;

	UPROPERTY(EditDefaultsOnly, Category = "Build|Placement")
	FName GroundActorTag = TEXT("Ground");

	bool bBuildMenuOpen = false;
};
