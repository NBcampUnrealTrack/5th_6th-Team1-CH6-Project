
#pragma once

#include "CoreMinimal.h"
#include "Building/BaseBuilding.h"
#include "AbilitySystemInterface.h"
#include "Common/FireStartInterface.h"
#include "Common/DataAssetInterface.h"
#include "Common/OnDeathInterface.h"
#include "BaseTurret.generated.h"

class UAbilitySystemComponent;
class URangedWeaponDataAsset;
class USphereComponent;
class UHealthAttributeSet;
class UGeometryCollection;
class UGeometryCollectionComponent;

UCLASS()
class BULLETANT_API ABaseTurret : public ABaseBuilding
								, public IAbilitySystemInterface
								, public IFireStartInterface
								, public IDataAssetInterface
								, public IOnDeathInterface
{
	GENERATED_BODY()
	
public:
	ABaseTurret();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaSeconds) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

protected:
	// IFireStartInterface
	virtual FVector GetFireStartLocation() const override;
	virtual FVector GetFireDirection() const override;

	// IDataAssetInterface
	virtual UDataAsset* GetDataAsset() const override;

	// IOnDeathInterface
	virtual void OnDeath() override;

	void GiveDefaultAbilities();
	void StartAutoFire();

	UFUNCTION()
	void Server_FireTick();

private:
	UFUNCTION()
	void OnTargetBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
	UFUNCTION()
	void OnTargetEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION()
	void UpdateCurrentTarget();
	
	UFUNCTION()
	void OnRep_Dead();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayDestruction(const FVector& ImpulseOrigin);

protected:
	UPROPERTY(VisibleAnywhere, Category = "Turret|Mesh")
	TObjectPtr<UStaticMeshComponent> BodyMesh;

	UPROPERTY(VisibleAnywhere, Category = "Turret|Mesh")
	TObjectPtr<UStaticMeshComponent> BarrelMesh;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UAbilitySystemComponent> ASC;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UHealthAttributeSet> HealthSet;

	UPROPERTY(EditDefaultsOnly, Category = "Turret|Data")
	TObjectPtr<URangedWeaponDataAsset> TurretData;

	UPROPERTY(EditDefaultsOnly, Category = "Turret|Socket")
	FName MuzzleSocketName = TEXT("Muzzle");

	FTimerHandle FireTimerHandle;

	UPROPERTY(VisibleAnywhere, Category = "Turret|Targeting")
	TObjectPtr<USphereComponent> TargetSerchingSphere;

	UPROPERTY(EditDefaultsOnly, Category = "Turret|Targeting")
	float SerchingSphereRadius = 1000.f;

	UPROPERTY()
	TArray<TWeakObjectPtr<AActor>> TargetCandidates;

	UPROPERTY(Replicated)
	TObjectPtr<AActor> CurrentTarget;

	UPROPERTY(EditDefaultsOnly, Category = "Turret|Targeting")
	float TargetSearchInterval = 1.f;

	FTimerHandle TargetSearchTimer;

	UPROPERTY(EditDefaultsOnly, Category = "Turret|Targeting")
	float TurnSpeedDegPerSec = 180.f;

	UPROPERTY(EditDefaultsOnly, Category = "Turret|Targeting")
	float PitchMin = -20.f;

	UPROPERTY(EditDefaultsOnly, Category = "Turret|Targeting")
	float PitchMax = 90.f;

	UPROPERTY(ReplicatedUsing = OnRep_Dead)
	bool bDead = false;

	UPROPERTY(EditDefaultsOnly, Category = "Turret|Destruction")
	TObjectPtr<UGeometryCollection> DestructionCollection;

	UPROPERTY(VisibleAnywhere, Category = "Turret|Destruction")
	TObjectPtr<UGeometryCollectionComponent> DestructionComp;
};
