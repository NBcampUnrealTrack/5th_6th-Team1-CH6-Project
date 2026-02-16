#pragma once

#include "CoreMinimal.h"
#include "Weapon/BaseWeapon.h"
#include "BaseJetpackWeapon.generated.h"

UCLASS()
class BULLETANT_API ABaseJetpackWeapon : public ABaseWeapon
{
	GENERATED_BODY()
	
public:
	virtual void BeginPlay() override;
};
