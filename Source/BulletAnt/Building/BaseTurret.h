
#pragma once

#include "CoreMinimal.h"
#include "Building/BaseBuilding.h"
#include "Common/FireStartInterface.h"
#include "Common/DataAssetInterface.h"
#include "BaseTurret.generated.h"

class URangedWeaponDataAsset;
class USphereComponent;
class ABaseEnemyCharacter;

UCLASS()
class BULLETANT_API ABaseTurret : public ABaseBuilding
								, public IFireStartInterface
								, public IDataAssetInterface
{
	GENERATED_BODY()
	
public:
	ABaseTurret();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaSeconds) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	// IFireStartInterface
	virtual FVector GetFireStartLocation_Implementation() const override;
	virtual FVector GetFireDirection_Implementation() const override;

	// IDataAssetInterface
	virtual UDataAsset* GetDataAsset() const override;

	// IOnDeathInterface
	virtual void OnDeath() override;
	virtual void OnRep_Dead() override;

private:
	UFUNCTION()
	void OnTargetBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
	UFUNCTION()
	void OnTargetEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION()
	void UpdateCurrentTarget();

protected:
	UPROPERTY(VisibleAnywhere, Category = "Turret|Mesh")
	TObjectPtr<UStaticMeshComponent> BodyMesh;

	UPROPERTY(VisibleAnywhere, Category = "Turret|Mesh")
	TObjectPtr<UStaticMeshComponent> BarrelMesh;

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
	TArray<TWeakObjectPtr<ABaseEnemyCharacter>> TargetCandidates;

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
};
