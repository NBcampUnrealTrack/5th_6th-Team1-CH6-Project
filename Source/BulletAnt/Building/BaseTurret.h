
#pragma once

#include "CoreMinimal.h"
#include "Building/BaseBuilding.h"
#include "AbilitySystemInterface.h"
#include "Common/FireStartInterface.h"
#include "Common/DataAssetInterface.h"
#include "BaseTurret.generated.h"

class UAbilitySystemComponent;
class URangedWeaponDataAsset;

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

protected:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UAbilitySystemComponent> ASC;

	UPROPERTY(EditDefaultsOnly, Category = "Turret|Data")
	TObjectPtr<URangedWeaponDataAsset> TurretData;

	UPROPERTY(EditDefaultsOnly, Category = "Turret|Socket")
	FName MuzzleSocketName = TEXT("Muzzle");

	FTimerHandle FireTimerHandle;
};
