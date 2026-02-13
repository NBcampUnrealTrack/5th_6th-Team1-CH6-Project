// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/BAAnimInstance.h"
#include "Player/BACharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "KismetAnimationLibrary.h"
#include "Kismet/KismetMathLibrary.h"

void UBAAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	APawn* Owner = TryGetPawnOwner();

	if (Owner)
	{
		Character = Cast<ABACharacter>(Owner);

		if (Character)
			Movement = Character->GetCharacterMovement();
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

	Direction = (GroundSpeed > 3.0f)
		? UKismetAnimationLibrary::CalculateDirection(Velocity, Rotation)
		: 0.f;

	float FinalPitch = Character->SyncAimPitch;
	float FinalYaw = Character->SyncAimYaw;

	FRotator AimRot = FRotator(FinalPitch, FinalYaw, 0.f);
	FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(AimRot, Character->GetActorRotation());

	AOPitch = FMath::Clamp(DeltaRot.Pitch, -90.0f, 90.0f);
	AOYaw = FMath::Clamp(DeltaRot.Yaw, -90.0f, 90.0f);

	bIsAiming = Character->bIsAiming;
	bIsTurning = Character->bIsTurning;
	
	bIsFalling = Movement->IsFalling();
	bIsCrouch = Character->bIsCrouched;
	bIsRunning = Character->bIsRunning;
	VerticalVelocity = Velocity.Z;
}