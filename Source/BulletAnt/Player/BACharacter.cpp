#include "Player/BACharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "AbilitySystemComponent.h"
#include "GAS/AttributeSet/HealthAttributeSet.h"
#include "Weapon/BaseWeapon.h"
#include "Weapon/Data/WeaponDataAsset.h"

ABACharacter::ABACharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	HealthAttributeSet = CreateDefaultSubobject<UHealthAttributeSet>(TEXT("HealthSet"));

	bIsAiming = false;
	bIsRunning = false;
}

void ABACharacter::BeginPlay()
{
	Super::BeginPlay();
}

void ABACharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (!EnhancedInputComponent) return;

	if (JumpAction)
	{
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
	}
	if (MoveAction)
	{
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ABACharacter::Move);
	}
	if (RunAction)
	{
		EnhancedInputComponent->BindAction(RunAction, ETriggerEvent::Started, this, &ABACharacter::StartRunning);
		EnhancedInputComponent->BindAction(RunAction, ETriggerEvent::Completed, this, &ABACharacter::StopRunning);
	}
	if (CrouchAction)
	{
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Started, this, &ABACharacter::CrouchInput);
	}
	if (LookAction)
	{
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ABACharacter::Look);
	}
	if (AttackAction)
	{
		EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Started, this, &ABACharacter::Attack);
	}
	if (AimAction)
	{
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Started, this, &ABACharacter::AimStart);
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Completed, this, &ABACharacter::AimStop);
	}
	if (InteractionAction)
	{
		EnhancedInputComponent->BindAction(InteractionAction, ETriggerEvent::Started, this, &ABACharacter::Interaction);
	}
}

void ABACharacter::Move(const FInputActionValue& Value)
{
	if (!Controller) return;

	FVector2D MovementVector = Value.Get<FVector2D>();
	const FRotator YawRotation(0, Controller->GetControlRotation().Yaw, 0);
	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	AddMovementInput(ForwardDirection, MovementVector.Y);
	AddMovementInput(RightDirection, MovementVector.X);
}

UDataAsset* ABACharacter::GetDataAsset() const
{
	return EquippedWeapon ? EquippedWeapon->GetWeaponData() : nullptr;
}

void ABACharacter::StartRunning(const FInputActionValue& Value)
{
	if (bIsCrouched) return;
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
		bIsRunning = false;
	}
	else
	{
		UnCrouch(false);
	}
	UpdateMovementSpeed();
}

void ABACharacter::Look(const FInputActionValue& Value)
{
	if (!Controller) return;

	FVector2D LookAxisVector = Value.Get<FVector2D>();
	AddControllerYawInput(LookAxisVector.X);
	AddControllerPitchInput(LookAxisVector.Y);
}

void ABACharacter::Attack(const FInputActionValue& Value)
{
	if (!AbilitySystemComponent) return;
	if (!EquippedWeapon) return;
	
	UWeaponDataAsset* WeaponData = EquippedWeapon->GetWeaponData();
	if (!WeaponData) return;

	FGameplayTagContainer Tag;
	Tag.AddTag(WeaponData->WeaponTag);

	AbilitySystemComponent->TryActivateAbilitiesByTag(Tag);
}

UAbilitySystemComponent* ABACharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void ABACharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);

		if (HasAuthority())
		{
			for (const TSubclassOf<UGameplayAbility>& AbilityClass : DefaultAbility)
			{
				if (AbilityClass)
				{
					FGameplayAbilitySpec Spec(AbilityClass, 1, -1, this);
					AbilitySystemComponent->GiveAbility(Spec);
				}
			}
		}

		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(HealthAttributeSet->GetHealthAttribute())
			.AddUObject(this, &ABACharacter::OnHealthChangedCallback);
	}

	if (DefaultWeaponClass)
	{
		EquipWeapon(DefaultWeaponClass);
	}
}

void ABACharacter::OnHealthChangedCallback(const FOnAttributeChangeData& Data) const
{
	OnHealthChanged.Broadcast(Data.NewValue, HealthAttributeSet->GetMaxHealth());
}

void ABACharacter::EquipWeapon(TSubclassOf<ABaseWeapon> WeaponClass)
{
	if (!HasAuthority()) return;
	if (!WeaponClass) return;

	if (EquippedWeapon)
	{
		EquippedWeapon->UnequipWeapon(AbilitySystemComponent);
		EquippedWeapon->Destroy();
		EquippedWeapon = nullptr;
	}

	FActorSpawnParameters Params;
	Params.Owner = this;
	Params.Instigator = this;

	ABaseWeapon* NewWeapon = GetWorld()->SpawnActor<ABaseWeapon>(
		WeaponClass,
		Params
	);

	if (!NewWeapon) return;

	NewWeapon->AttachToComponent(
		GetMesh(),
		FAttachmentTransformRules::SnapToTargetNotIncludingScale,
		TEXT("WeaponSocket")
	);

	NewWeapon->EquipWeapon(AbilitySystemComponent);

	EquippedWeapon = NewWeapon;
}

void ABACharacter::AimStart(const FInputActionValue& Value)
{
	bIsAiming = true;
	bUseControllerRotationYaw = true;
	GetCharacterMovement()->bOrientRotationToMovement = false;
	UpdateMovementSpeed();
}

void ABACharacter::AimStop(const FInputActionValue& Value)
{
	bIsAiming = false;
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	UpdateMovementSpeed();
}

void ABACharacter::Interaction(const FInputActionValue& Value)
{
}

void ABACharacter::UpdateMovementSpeed()
{
	float NewSpeed = WalkSpeed;

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
