#include "Player/BAAnimInstance.h"
#include "Player/BACharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "KismetAnimationLibrary.h"

void UBAAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	APawn* Owner = TryGetPawnOwner();
	if (!Owner) return;

	Character = Cast<ABACharacter>(Owner);
	if (Character)
	{
		Movement = Character->GetCharacterMovement();
	}
}

void UBAAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (!Character || !Movement) return;

	FVector Velocity = Movement->Velocity;
	GroundSpeed = Velocity.Size2D();

	FRotator Rotation = Character->GetActorRotation();
	Direction = (GroundSpeed > 3.0f) 
		? UKismetAnimationLibrary::CalculateDirection(Velocity, Rotation) 
		: 0.0f;

	bIsAiming = Character->bIsAiming;
	bIsFalling = Movement->IsFalling();
	bIsRunning = Character->bIsRunning;
	VerticalVelocity = Velocity.Z;
}