#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BATransportShip.generated.h"

class UProjectileMovementComponent;
class ABAItemBox;
class ABaseWeapon;

UCLASS()
class BULLETANT_API ABATransportShip : public AActor
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	USceneComponent* Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* PlaneMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UProjectileMovementComponent* ProjectileMovement;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<ABAItemBox> ItemBox;
	
public:	
	ABATransportShip();

	UFUNCTION()
	void InitPlane(FVector& InDropLocation, TSubclassOf<ABaseWeapon> InItem);

protected:
	virtual void Tick(float DeltaTime) override;

	void SpawnItemBox();
	
	UPROPERTY()
	TSubclassOf<ABaseWeapon> Item;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Speed = 1000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TotalDistance = 40000.f;

	float CurrentDistance = 0.f;
	bool bIsDropped = false;
};
