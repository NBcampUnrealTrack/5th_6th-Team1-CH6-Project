// Fill out your copyright notice in the Description page of Project Settings.

#include "Player/BACharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

ABACharacter::ABACharacter()
{
	// Tick ??
	PrimaryActorTick.bCanEverTick = true;

	// ??? ?? ??
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// ?? ???? ??
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;

	// ??? ? ?? ? ??
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;

	// ??? ?? ? ??
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	// ?? ?? ???
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;

	bIsAiming = false;
	bIsRunning = false;
}

void ABACharacter::BeginPlay()
{
	Super::BeginPlay();
}

void ABACharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// ?? ???
void ABACharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// ??
		if (JumpAction)
		{
			EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
			EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
		}

		// ??
		if (MoveAction)
		{
			EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ABACharacter::Move);
		}

		// ???
		if (RunAction)
		{
			EnhancedInputComponent->BindAction(RunAction, ETriggerEvent::Started, this, &ABACharacter::StartRunning);
			EnhancedInputComponent->BindAction(RunAction, ETriggerEvent::Completed, this, &ABACharacter::StopRunning);
		}

		// ??
		if (CrouchAction)
		{
			EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Started, this, &ABACharacter::CrouchInput);
		}

		// ?? ?? (???)
		if (LookAction)
		{
			EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ABACharacter::Look);
		}

		// ??
		if (AimAction)
		{
			EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Started, this, &ABACharacter::AimStart);
			EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Completed, this, &ABACharacter::AimStop);
		}

		// ????
		if (InteractionAction)
		{
			EnhancedInputComponent->BindAction(InteractionAction, ETriggerEvent::Started, this, &ABACharacter::Interaction);
		}
	}
}

// ?? ?? ??
void ABACharacter::Move(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// ????? ?? ?? ??(Yaw)? ???
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// ?? ?? (W/S) ??
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// ?? ?? (A/D) ??
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// ?? ??
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void ABACharacter::StartRunning(const FInputActionValue& Value)
{
	// ?? ????? ??? ??
	if (bIsCrouched)
	{
		return;
	}

	bIsRunning = true;
	UpdateMovementSpeed();
}

void ABACharacter::StopRunning(const FInputActionValue& Value)
{
	bIsRunning = false;
	UpdateMovementSpeed();
}

void ABACharacter::CrouchInput(const FInputActionValue& Value)
{
	if (!bIsCrouched)
	{
		Crouch(false);
		bIsRunning = false;  // ??? ??? ??
	}
	else
	{
		UnCrouch(false);
	}

	UpdateMovementSpeed();
}

// ?? ?? ?? ??
void ABACharacter::Look(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

// ?? ??
void ABACharacter::AimStart(const FInputActionValue& Value)
{
	bIsAiming = true;

	// ??? ?? ??
	bUseControllerRotationYaw = true;
	GetCharacterMovement()->bOrientRotationToMovement = false;

	UpdateMovementSpeed();
}

// ?? ?
void ABACharacter::AimStop(const FInputActionValue& Value)
{
	bIsAiming = false;

	// ??? ?? ?? ??
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;

	UpdateMovementSpeed();
}

void ABACharacter::Interaction(const FInputActionValue& Value)
{
	// TODO: ???? ?? ??
}

void ABACharacter::UpdateMovementSpeed()
{
	float NewSpeed = WalkSpeed;

	// ????: ?? > ?? > ??? > ??
	if (bIsCrouched)
	{
		NewSpeed = CrouchSpeed;
	}
	else if (bIsAiming)
	{
		NewSpeed = AimSpeed;
	}
	else if (bIsRunning)
	{
		NewSpeed = RunningSpeed;
	}

	GetCharacterMovement()->MaxWalkSpeed = NewSpeed;
}
