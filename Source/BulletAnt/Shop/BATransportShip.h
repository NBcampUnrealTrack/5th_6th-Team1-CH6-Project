#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BATransportShip.generated.h"

class USplineComponent;
class UTimelineComponent;
class ABAItemBox;

UCLASS()
class BULLETANT_API ABATransportShip : public AActor
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	USceneComponent* Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* PlaneMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	USplineComponent* Spline;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<ABAItemBox> ItemBox;
	
public:	
	ABATransportShip();

	UFUNCTION()
	void InitPlane(FVector& InDropLocation, TSubclassOf<AActor> InItem);

protected:
	virtual void Tick(float DeltaTime) override;

	void SpawnItemBox();
	
	UPROPERTY()
	TSubclassOf<AActor> Item;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Speed = 1000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TotalDistance = 20000.f;

	float CurrentDistance = 0.f;
	float DropDistance = 0.f;
	bool bIsDropped = false;
};
