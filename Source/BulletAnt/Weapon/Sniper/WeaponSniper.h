
#pragma once

#include "CoreMinimal.h"
#include "Weapon/BaseRangedWeapon.h"
#include "WeaponSniper.generated.h"

class USceneCaptureComponent2D;

UCLASS()
class BULLETANT_API AWeaponSniper : public ABaseRangedWeapon
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components", meta = (AllowPrivateAccess = "true"))
	USceneCaptureComponent2D* SceneCapture;

public:
	AWeaponSniper();

protected:

	virtual void BeginPlay() override;
};
