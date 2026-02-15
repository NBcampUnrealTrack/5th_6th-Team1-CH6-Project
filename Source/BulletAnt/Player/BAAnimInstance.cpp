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

	Direction = (GroundSpeed > 3.f)
		? UKismetAnimationLibrary::CalculateDirection(Velocity, Rotation)
		: 0.f;

	float FinalPitch = Character->SyncAimPitch;
	float FinalYaw = Character->SyncAimYaw;

	FRotator AimRot = FRotator(FinalPitch, FinalYaw, 0.f);
	FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(AimRot, Rotation);
	if (bIsAiming)
	{
		DeltaRot = CameraTargetOffset();
	}
	TargetPitch = FMath::Clamp(DeltaRot.Pitch, -90.0f, 90.0f);
	TargetYaw = FMath::Clamp(DeltaRot.Yaw, -90.0f, 90.0f);

	AOPitch = FMath::FInterpTo(AOPitch, TargetPitch, DeltaSeconds, 15.0f);
    AOYaw = FMath::FInterpTo(AOYaw, TargetYaw, DeltaSeconds, 15.0f);

	bIsAiming = Character->bIsAiming;
	bIsTurning = Character->bIsTurning;
	
	bIsFalling = Movement->IsFalling();
	bIsCrouch = Character->bIsCrouched;
	bIsRunning = Character->bIsRunning;
	VerticalVelocity = Velocity.Z;

	RootYawOffset = Character->RootYawOffset * -1;
}

FRotator UBAAnimInstance::CameraTargetOffset()
{
	if (!Character || !Character->GetController())
		return FRotator::ZeroRotator;
	FVector CamLoc; FRotator CamRot;
	Character->GetController()->GetPlayerViewPoint(CamLoc, CamRot);

	FVector TraceEnd = CamLoc + (CamRot.Vector() * 5000.0f);

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Character);

	bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, CamLoc, TraceEnd, ECC_Visibility, Params);

	FVector TargetLocation = bHit ? Hit.ImpactPoint : TraceEnd;

	FVector StartLocation = Character->GetMesh()->GetSocketLocation("head");

	FRotator LookAtRot = UKismetMathLibrary::FindLookAtRotation(StartLocation, TargetLocation);

	FRotator ActorRot = Character->GetActorRotation();
	FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(LookAtRot, ActorRot);

	return DeltaRot;
}
