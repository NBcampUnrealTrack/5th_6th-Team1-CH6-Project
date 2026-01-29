#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "BAAnimInstance.generated.h"

class ABACharacter;
class UCharacterMovementComponent;

UCLASS()
class BULLETANT_API UBAAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Character")
	TObjectPtr<ABACharacter> Character;

	UPROPERTY(BlueprintReadOnly, Category = "Character")
	TObjectPtr<UCharacterMovementComponent> Movement;

	UPROPERTY(BlueprintReadOnly, Category = "Character")
	float GroundSpeed;

	UPROPERTY(BlueprintReadOnly, Category = "Character")
	float Direction;

	UPROPERTY(BlueprintReadOnly, Category = "Character")
	bool bIsAiming;

	UPROPERTY(BlueprintReadOnly, Category = "Character")
	bool bIsFalling;

	UPROPERTY(BlueprintReadOnly, Category = "Character")
	bool bIsRunning;

	UPROPERTY(BlueprintReadOnly, Category = "Character")
	float VerticalVelocity;
};
