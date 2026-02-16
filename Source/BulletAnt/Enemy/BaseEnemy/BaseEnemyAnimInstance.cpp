// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/BaseEnemy/BaseEnemyAnimInstance.h"
#include "Enemy/BaseEnemy/BaseEnemyCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "KismetAnimationLibrary.h"

void UBaseEnemyAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	APawn* Owner = TryGetPawnOwner();
	if (!IsValid(Owner))
	{
		return;
	}

	Enemy = Cast<ABaseEnemyCharacter>(Owner);
	if (!IsValid(Enemy))
	{
		return;
	}

	MovementComponent = Enemy->GetCharacterMovement();
}

void UBaseEnemyAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (!IsValid(Enemy) || !IsValid(MovementComponent))
	{
		return;
	}

	Velocity = MovementComponent->Velocity;
	GroundSpeed = Velocity.Size2D();
	if (GroundSpeed > 0.01)
	{
		bShouldMove = true;
	}

	bIsTurning = Enemy->bIsTurning;
	bIsTurningLeft = Enemy->bIsTurningLeft;

	bIsFalling = MovementComponent->IsFalling();
}
