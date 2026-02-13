#pragma once

#include "CoreMinimal.h"
#include "Weapon/BaseWeapon.h"
#include "BaseMiningWeapon.generated.h"


UCLASS()
class BULLETANT_API ABaseMiningWeapon : public ABaseWeapon
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;
};
