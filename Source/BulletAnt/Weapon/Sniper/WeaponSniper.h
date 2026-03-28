
#pragma once

#include "CoreMinimal.h"
#include "Weapon/BaseRangedWeapon.h"
#include "WeaponSniper.generated.h"

class USceneCaptureComponent2D;
class UTextureRenderTarget2D;
class ABACharacter;

UCLASS()
class BULLETANT_API AWeaponSniper : public ABaseRangedWeapon
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components", meta = (AllowPrivateAccess = "true"))
	USceneCaptureComponent2D* SceneCapture;

public:
	AWeaponSniper();

	FORCEINLINE USceneCaptureComponent2D* GetSceneCapture() { return SceneCapture; };

	void StartNightVision();
	void StopNightVision();

protected:

	UFUNCTION()
	void SceneCaptureHideArrowMesh(ABACharacter* Player);
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly)
	UTextureRenderTarget2D* RT;

	UPROPERTY(EditDefaultsOnly)
	UMaterialInstance* NightVisionMaterial;

	UPROPERTY()
	UMaterialInstanceDynamic* NightVisionMID;

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	USoundBase* NightVisionOnSound;

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	USoundBase* NightVisionOffSound;
};
