#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "PMWCharacter.generated.h"

class UCameraComponent;
class USpringArmComponent;
struct FInputActionValue;
class UInputAction;

UCLASS()
class BULLETANT_API APMWCharacter : public ACharacter
{
	GENERATED_BODY()

#pragma region Character Override

public:
	APMWCharacter();

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	virtual void BeginPlay() override;

#pragma endregion

#pragma region Components

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components|Viewport", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> Camera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components|Viewport", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpringArmComponent> SpringArm;

#pragma endregion

#pragma region InputAction

protected:
	UFUNCTION()
	void Input_Move(const FInputActionValue& Value);
	UFUNCTION()
	void Input_Look(const FInputActionValue& Value);
	UFUNCTION()
	void Input_LeftClick(const FInputActionValue& Value);

protected:
	UPROPERTY(EditAnywhere)
	TObjectPtr<UInputAction> IA_Move;
	UPROPERTY(EditAnywhere)
	TObjectPtr<UInputAction> IA_Look;
	UPROPERTY(EditAnywhere)
	TObjectPtr<UInputAction> IA_Jump;
	UPROPERTY(EditAnywhere)
	TObjectPtr<UInputAction> IA_LeftClick;

#pragma endregion

#pragma region Mining

protected:
	void ExecuteMining();
	UFUNCTION()
	void EnableMining();

protected:
	FTimerHandle MiningTimerHandle;

	uint8 bCanMine : 1 = true;
	const float MiningCooldown = 0.5f;

#pragma endregion
};
