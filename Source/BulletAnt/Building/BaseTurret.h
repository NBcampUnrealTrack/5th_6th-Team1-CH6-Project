
#pragma once

#include "CoreMinimal.h"
#include "Building/BaseBuilding.h"
#include "AbilitySystemInterface.h"
#include "Common/FireStartInterface.h"
#include "Common/DataAssetInterface.h"
#include "BaseTurret.generated.h"

class UAbilitySystemComponent;
class URangedWeaponDataAsset;
class USphereComponent;

UCLASS()
class BULLETANT_API ABaseTurret : public ABaseBuilding
								, public IAbilitySystemInterface
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
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

protected:
	// IFireStartInterface
	virtual FVector GetFireStartLocation() const override;
	virtual FVector GetFireDirection() const override;

	// IDataAssetInterface
	virtual UDataAsset* GetDataAsset() const override;

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

protected:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UAbilitySystemComponent> ASC;

	UPROPERTY(EditDefaultsOnly, Category = "Turret|Data")
	TObjectPtr<URangedWeaponDataAsset> TurretData;

	UPROPERTY(EditDefaultsOnly, Category = "Turret|Socket")
	FName MuzzleSocketName = TEXT("Muzzle");

	FTimerHandle FireTimerHandle;

	UPROPERTY(VisibleAnywhere, Category = "Turret|Targeting")
	TObjectPtr<USphereComponent> TargetSerchingSphere;

	UPROPERTY(EditDefaultsOnly, Category = "Turret|Targeting")
	float SerchingSphereRadius = 300.f;

	UPROPERTY()
	TArray<TWeakObjectPtr<AActor>> TargetCandidates;

	UPROPERTY()
	TWeakObjectPtr<AActor> CurrentTarget;

	UPROPERTY(EditDefaultsOnly, Category = "Turret|Targeting")
	float TargetSearchInterval = 1.f;

	FTimerHandle TargetSearchTimer;

	UPROPERTY(EditDefaultsOnly, Category = "Turret|Targeting")
	float TurnSpeedDegPerSec = 180.f;
};
