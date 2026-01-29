#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Building/BuildingData.h"
#include "BuildManagerComponent.generated.h"

class ABuildPreview;
class UBuildingData;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class BULLETANT_API UBuildManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UBuildManagerComponent();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable)
	void EnterBuildMode();

	UFUNCTION(BlueprintCallable)
	void ExitBuildMode();

	UFUNCTION(BlueprintCallable)
	void TryPlace();

	bool IsBuildMode() const { return bBuildMode; }

private:
	bool CheckCanPlaceAt(const FVector& Location, float Radius) const;
	void RefreshCachedReferences();

	UPROPERTY(EditDefaultsOnly, Category = "Build|Test")
	TObjectPtr<UBuildingData> DefaultBuildData;

	UPROPERTY(EditDefaultsOnly, Category = "Build")
	TSubclassOf<ABuildPreview> PreviewActorClass;

	UPROPERTY()
	TObjectPtr<UBuildingData> CurrentData;

	UPROPERTY()
	TObjectPtr<ABuildPreview> PreviewActor;

	TWeakObjectPtr<APlayerController> CachedPC;

	bool bBuildMode = false;
};
