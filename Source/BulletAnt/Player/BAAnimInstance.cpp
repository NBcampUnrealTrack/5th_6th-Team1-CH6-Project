// Fill out your copyright notice in the Description page of Project Settings.

#include "Player/BAAnimInstance.h"
#include "Player/BACharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "KismetAnimationLibrary.h"

void UBAAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	APawn* Owner = TryGetPawnOwner();

	if (Owner)
	{
		Character = Cast<ABACharacter>(Owner);

		if (Character)
		{
			Movement = Character->GetCharacterMovement();
		}
	}
}

void UBAAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	// ??? ??? ??
	if (Character == nullptr || Movement == nullptr)
	{
		return;
	}

	// UCharacterMovementComponent?? Velocity ?? ????
	FVector Velocity = Movement->Velocity;

	GroundSpeed = Velocity.Size2D();
	VerticalVelocity = Velocity.Z;

	FRotator Rotation = Character->GetActorRotation();

	if (GroundSpeed > 3.0f)
	{
		Direction = UKismetAnimationLibrary::CalculateDirection(Velocity, Rotation);
	}
	else
	{
		Direction = 0.0f;
	}

	bIsAiming = Character->bIsAiming;
	bIsRunning = Character->bIsRunning;
	bIsFalling = Movement->IsFalling();
	bIsCrouched = Character->bIsCrouched;
}
