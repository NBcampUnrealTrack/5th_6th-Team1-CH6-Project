// Fill out your copyright notice in the Description page of Project Settings.

#include "Player/ParkourComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

UParkourComponent::UParkourComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UParkourComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (OwnerCharacter)
	{
		MovementComponent = OwnerCharacter->GetCharacterMovement();
	}
}

void UParkourComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bIsClimbing) return;
	if (!OwnerCharacter) return;

	ClimbTimer += DeltaTime;

	float Alpha = ClimbTimer / ClimbDuration;
	FVector NewLocation = FMath::Lerp(ClimbStartLocation, ClimbEndLocation, Alpha);

	OwnerCharacter->SetActorLocation(NewLocation);

	if (Alpha >= 1.f)
	{
		EndClimb();
	}
}

void UParkourComponent::StartClimb(FVector TargetLocation)
{
	if (bIsClimbing) return;
	if (!OwnerCharacter || !MovementComponent) return;

	MovementComponent->SetMovementMode(MOVE_Flying);

	bIsClimbing = true;
	ClimbStartLocation = OwnerCharacter->GetActorLocation();
	ClimbEndLocation = TargetLocation;
	ClimbTimer = 0.f;
}

void UParkourComponent::EndClimb()
{
	if (!MovementComponent) return;

	bIsClimbing = false;
	MovementComponent->SetMovementMode(MOVE_Walking);
}
