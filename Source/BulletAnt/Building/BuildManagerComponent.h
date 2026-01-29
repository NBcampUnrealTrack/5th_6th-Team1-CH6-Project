// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Building/BuildingData.h"
#include "BuildManagerComponent.generated.h"

class ABuildPreview;
class UBuildingData;

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

	bool IsBuildMode() const { return bBuildMode; }

private:
	bool CheckCanPlaceAt(const FVector& Location, float Radius) const;

private:
	UPROPERTY(EditDefaultsOnly, Category = "Build|Test")
	UBuildingData* DefaultBuildData = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Build")
	TSubclassOf<ABuildPreview> PreviewActorClass;

	UPROPERTY()
	UBuildingData* CurrentData = nullptr;

	UPROPERTY()
	ABuildPreview* PreviewActor = nullptr;

	bool bBuildMode = false;
};
