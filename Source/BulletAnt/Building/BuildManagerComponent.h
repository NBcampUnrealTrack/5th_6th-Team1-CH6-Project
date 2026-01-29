// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "Building/BuildingRow.h"
#include "BuildManagerComponent.generated.h"

class ABuildPreview;

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
	void RotatePreviewByWheel(float WheelAxisValue);

	bool IsBuildMode() const { return bBuildMode; }

private:
	bool ComputePreviewPlacement(FVector& OutLocation, FRotator& OutRotation, bool& bOutHasValidSurface);

	UFUNCTION(Server, Reliable)
	void ServerTryPlace(FName BuildingRow, const FVector& Location, const FRotator& Rotation);

	bool CheckCanPlaceAt(const FVector& Location, const FRotator& Rotation, const FVector& InBoxExtent) const;

	void RefreshCachedRef();

	void SetCurrentBuildingRow(FName NewRow);

private:
	UPROPERTY(EditDefaultsOnly, Category = "Build|Data")
	TObjectPtr<UDataTable> BuildingTable;

	UPROPERTY(EditDefaultsOnly, Category = "Build|Data")
	TSubclassOf<ABuildPreview> PreviewActorClass;

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
};
