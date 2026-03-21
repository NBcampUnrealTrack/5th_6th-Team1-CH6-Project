#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BATransportShip.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDropFromPlane);

class UProjectileMovementComponent;
class ABAItemBox;
class ABaseWeapon;
class UNiagaraComponent;
class UAudioComponent;

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

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UNiagaraComponent* TrailEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UAudioComponent* EngineSound;
	
public:	
	ABATransportShip();

	FORCEINLINE UStaticMeshComponent* GetMesh() { return PlaneMesh; };

	UFUNCTION()
	void InitItemPlane(FVector& InDropLocation, TSubclassOf<ABaseWeapon> InItem);

	UFUNCTION()
	void InitPlayerPlane(FVector& InDropLocation, ACharacter* PlayerCharacter);

	UFUNCTION()
	void HandleDropFromPlane();

	UPROPERTY(BlueprintAssignable)
	FDropFromPlane DropFromPlane;

protected:
	virtual void Tick(float DeltaTime) override;

	void SpawnItemBox();
	
	UPROPERTY()
	TSubclassOf<ABaseWeapon> Item = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Speed = 1000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TotalDistance = 40000.f;

	TWeakObjectPtr<ACharacter> CachedPlayerCharacter;

	float CurrentDistance = 0.f;
	bool bIsDropped = false;
	bool bIsPlayer = false;
};
