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
	//캐릭터 없으면 nullptr 반환
	if (Character == nullptr || Movement == nullptr) return;

	//UCharacterMovementComponent에서 Velocity 변수 가져오기
	FVector Velocity = Movement->Velocity;

	GroundSpeed = Velocity.Size2D();

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
	
	bIsFalling = Movement->IsFalling();
	VerticalVelocity = Velocity.Z;
}