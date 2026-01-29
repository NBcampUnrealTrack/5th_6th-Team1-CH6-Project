#pragma once

#include "CoreMinimal.h"
#include "Weapon/BaseWeapon.h"
#include "Common/FireStartInterface.h"
#include "BaseRangedWeapon.generated.h"

UCLASS()
class BULLETANT_API ABaseRangedWeapon : public ABaseWeapon, public IFireStartInterface
{
	GENERATED_BODY()

public:
	virtual FVector GetFireStartLocation() const override;
	virtual FVector GetFireDirection() const override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	FName MuzzleSocketName = TEXT("Muzzle");
};
