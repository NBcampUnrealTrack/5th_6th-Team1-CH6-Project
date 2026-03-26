
#pragma once

#include "CoreMinimal.h"
#include "Weapon/BaseRangedWeapon.h"
#include "WeaponSniper.generated.h"

class USceneCaptureComponent2D;
class UTextureRenderTarget2D;

UCLASS()
class BULLETANT_API AWeaponSniper : public ABaseRangedWeapon
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components", meta = (AllowPrivateAccess = "true"))
	USceneCaptureComponent2D* SceneCapture;

public:
	AWeaponSniper();

	FORCEINLINE USceneCaptureComponent2D* GetSceneCapture() { return SceneCapture; };

protected:

	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly)
	UTextureRenderTarget2D* RT;

};
