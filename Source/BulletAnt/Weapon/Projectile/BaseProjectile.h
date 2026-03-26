#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BaseProjectile.generated.h"

class UProjectileMovementComponent;
class USphereComponent;
class URangedWeaponDataAsset;
class UStaticMeshComponent;
class UNiagaraComponent;

UCLASS()
class BULLETANT_API ABaseProjectile : public AActor
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	USphereComponent* CollisionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UProjectileMovementComponent* ProjectileMovement;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* BulletMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UNiagaraComponent* Tracer;

	
public:	
	ABaseProjectile();

	UFUNCTION(BlueprintCallable)
	void InitProjectile(
		const FVector& Start,
		const FVector& Direction,
		const float Radius,
		float Speed,
		float Damage,
		URangedWeaponDataAsset* Data,
		AActor* InOwner
	);

	UFUNCTION(BlueprintCallable)
	void ActivateProjectile();

	UFUNCTION(BlueprintCallable)
	void DeactivateProjectile();

	FORCEINLINE bool GetbIsActive() { return bIsActive; };

protected:
	UFUNCTION()
	virtual void OnHit(
		UPrimitiveComponent* HitComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		FVector NormalImpulse,
		const FHitResult& Hit
	);

	UPROPERTY(EditAnywhere, Category = "Projectile|Hit", meta = (ClampMin = 0, ClampMax = 50000))
	float PhysicsForce = 100.0f;

	float CachedDamage;

	UPROPERTY()
	AActor* CachedOwner;

	UPROPERTY()
	TObjectPtr<URangedWeaponDataAsset> CachedData;

	UPROPERTY()
	bool bIsActive = false;

	

};
