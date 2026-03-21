#include "Mining/Temp/PMWCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Mining/Temp/PMWPlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "Mining/VoxelGround.h"

APMWCharacter::APMWCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	if (GetMesh())
	{
		GetMesh()->bEnableUpdateRateOptimizations = false;
		GetMesh()->bNoSkeletonUpdate = false;
		GetMesh()->bPauseAnims = false;
		GetMesh()->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
	}

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->bUseControllerDesiredRotation = false;
	GetCharacterMovement()->MaxWalkSpeed = 600.0f;

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->TargetArmLength = 500.0f;
	SpringArm->bUsePawnControlRotation = true;
	SpringArm->bInheritYaw = true;
	SpringArm->bInheritPitch = true;
	SpringArm->bInheritRoll = false;
	SpringArm->bDoCollisionTest = false;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = false;
}

void APMWCharacter::BeginPlay()
{
	Super::BeginPlay();

}

void APMWCharacter::Input_Move(const FInputActionValue& Value)
{
	if (!IsValid(Controller))
		return;

	const FVector2D InMovementVector = Value.Get<FVector2D>();

	const FRotator Rotation = Camera->GetComponentRotation();
	const FRotator YawRotation(0, Rotation.Yaw, 0);

	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	AddMovementInput(ForwardDirection, InMovementVector.X);
	AddMovementInput(RightDirection, InMovementVector.Y);
}

void APMWCharacter::Input_Look(const FInputActionValue& InputValue)
{
	FVector2D LookVector = InputValue.Get<FVector2D>();

	AddControllerYawInput(LookVector.X);
	AddControllerPitchInput(LookVector.Y);
}

void APMWCharacter::Input_LeftClick(const FInputActionValue& Value)
{
	if (bCanMine == false)
		return;

	ExecuteMining();
}

void APMWCharacter::ExecuteMining()
{
	FVector Start = Camera->GetComponentLocation();
	FVector End = Start + Camera->GetForwardVector() * 700.0f;

	FHitResult HitResult;
	bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility);
	if (bHit)
	{
		AActor* HitActor = HitResult.GetActor();
		if (HitActor)
		{
			AVoxelGround* Ground = Cast<AVoxelGround>(HitActor);
			if (!Ground) Ground = Cast<AVoxelGround>(HitActor->GetOwner());

			if (IsValid(Ground) == true)
			{
				//Ground->DigGround(HitResult.Location, 180.0f);

				bCanMine = false;
				GetWorldTimerManager().SetTimer(
					MiningTimerHandle,
					this,
					&ThisClass::EnableMining,
					MiningCooldown);
			}
		}
	}
}

void APMWCharacter::EnableMining()
{
	GetWorldTimerManager().ClearTimer(MiningTimerHandle);
	bCanMine = true;
}

void APMWCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (IsValid(EnhancedInputComponent) == true)
	{
		EnhancedInputComponent->BindAction(IA_Move, ETriggerEvent::Triggered, this, &ThisClass::Input_Move);
		EnhancedInputComponent->BindAction(IA_Look, ETriggerEvent::Triggered, this, &ThisClass::Input_Look);
		EnhancedInputComponent->BindAction(IA_Jump, ETriggerEvent::Started, this, &ThisClass::Jump);
		EnhancedInputComponent->BindAction(IA_Jump, ETriggerEvent::Completed, this, &ThisClass::StopJumping);
		EnhancedInputComponent->BindAction(IA_LeftClick, ETriggerEvent::Ongoing, this, &ThisClass::Input_LeftClick);
	}
}
