#pragma once

#include "CoreMinimal.h"
#include "Weapon/Projectile/BaseProjectile.h"
#include "MissileProjectile.generated.h"

class UNiagaraSystem;
class USoundBase;
class URangedWeaponDataAsset;

UCLASS()
class BULLETANT_API AMissileProjectile : public ABaseProjectile
{
	GENERATED_BODY()

public:
	AMissileProjectile();

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void HandleActorHit(AActor* SelfActor, AActor* OtherActor, FVector NormalImpulse, const FHitResult& Hit);
	void Explode(const FHitResult& Hit);
	void ApplyExplosionDamage(const FVector& ExplosionLocation, AActor* DirectHitActor);

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Missile|Explosion", meta = (ClampMin = "0.0"))
	float ExplosionRadius = 300.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Missile|Explosion", meta = (ClampMin = "0.0"))
	float ExplosionDamage = 50.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Missile|Explosion")
	TObjectPtr<UNiagaraSystem> ExplosionEffect = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Missile|Explosion")
	TObjectPtr<USoundBase> ExplosionSound = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Missile|Explosion")
	FVector ExplosionEffectScale = FVector(1.f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Missile|Explosion", meta = (ClampMin = "0.0"))
	float ExplosionOffsetAlongNormal = 10.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Missile|Explosion|Debug")
	bool bDrawExplosionDebug = false;

private:
	bool bExplosionProcessed = false;
};