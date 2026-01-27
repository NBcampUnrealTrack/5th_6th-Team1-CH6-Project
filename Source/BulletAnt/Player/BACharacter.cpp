// Fill out your copyright notice in the Description page of Project Settings.

#include "Player/BACharacter.h"
#include "Player/BAAttributeSet.h"
#include "Weapon/BaseWeapon.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "AbilitySystemComponent.h"
#include "Components/SkeletalMeshComponent.h"

ABACharacter::ABACharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    // Rotation: Character does not follow controller rotation (only camera follows)
    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;

    // Movement: Character rotates toward movement direction
    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);

    // Create Spring Arm
    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->TargetArmLength = 400.0f;
    CameraBoom->bUsePawnControlRotation = true;

    // Create Camera
    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
    FollowCamera->bUsePawnControlRotation = false;

    // Create GAS Component
    AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));

    // Create AttributeSet
    AttributeSet = CreateDefaultSubobject<UBAAttributeSet>(TEXT("AttributeSet"));
}

UAbilitySystemComponent* ABACharacter::GetAbilitySystemComponent() const
{
    return AbilitySystemComponent;
}

void ABACharacter::BeginPlay()
{
    Super::BeginPlay();

    // Spawn default weapon
    if (DefaultWeaponClass)
    {
        ABaseWeapon* SpawnedWeapon = GetWorld()->SpawnActor<ABaseWeapon>(DefaultWeaponClass);
        if (SpawnedWeapon)
        {
            EquipWeapon(SpawnedWeapon);
        }
    }
}

void ABACharacter::PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);

    // Initialize ASC on server
    if (AbilitySystemComponent)
    {
        AbilitySystemComponent->InitAbilityActorInfo(this, this);
        InitializeAttributes();
    }
}

void ABACharacter::InitializeAttributes()
{
    if (!AbilitySystemComponent || !AttributeSet)
    {
        return;
    }

    // Initial attributes can be set via GameplayEffect or directly
    // Using AttributeSet default values
}

void ABACharacter::EquipWeapon(ABaseWeapon* NewWeapon)
{
    if (!NewWeapon) return;

    // Unequip current weapon
    if (CurrentWeapon)
    {
        UnequipWeapon();
    }

    CurrentWeapon = NewWeapon;

    // Attach weapon to character mesh
    FAttachmentTransformRules AttachRules(EAttachmentRule::SnapToTarget, true);
    CurrentWeapon->AttachToComponent(GetMesh(), AttachRules, WeaponSocketName);

    // Grant weapon abilities
    if (AbilitySystemComponent)
    {
        CurrentWeapon->EquipWeapon(AbilitySystemComponent);
    }
}

void ABACharacter::UnequipWeapon()
{
    if (!CurrentWeapon) return;

    // Remove weapon abilities
    if (AbilitySystemComponent)
    {
        CurrentWeapon->UnequipWeapon(AbilitySystemComponent);
    }

    // Detach and destroy weapon
    CurrentWeapon->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
    CurrentWeapon->Destroy();
    CurrentWeapon = nullptr;
}

void ABACharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void ABACharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
    {
        // Jump
        if (JumpAction)
        {
            EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
            EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
        }

        // Move
        if (MoveAction)
        {
            EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ABACharacter::Move);
        }

        // Look (Mouse)
        if (LookAction)
        {
            EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ABACharacter::Look);
        }

        // Fire
        if (FireAction)
        {
            EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Started, this, &ABACharacter::StartFire);
            EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Completed, this, &ABACharacter::StopFire);
        }
    }
}

void ABACharacter::Move(const FInputActionValue& Value)
{
    FVector2D MovementVector = Value.Get<FVector2D>();
    if (Controller != nullptr)
    {
        const FRotator Rotation = Controller->GetControlRotation();
        const FRotator YawRotation(0, Rotation.Yaw, 0);

        const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
        const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

        AddMovementInput(ForwardDirection, MovementVector.Y);
        AddMovementInput(RightDirection, MovementVector.X);
    }
}

void ABACharacter::Look(const FInputActionValue& Value)
{
    FVector2D LookAxisVector = Value.Get<FVector2D>();

    if (Controller != nullptr)
    {
        AddControllerYawInput(LookAxisVector.X);
        AddControllerPitchInput(LookAxisVector.Y);
    }
}

void ABACharacter::StartFire(const FInputActionValue& Value)
{
    if (!AbilitySystemComponent) return;

    // Activate ability by tag
    if (FireAbilityTag.IsValid())
    {
        FGameplayTagContainer TagContainer;
        TagContainer.AddTag(FireAbilityTag);
        AbilitySystemComponent->TryActivateAbilitiesByTag(TagContainer);
    }
}

void ABACharacter::StopFire(const FInputActionValue& Value)
{
    // Add fire stop logic if needed
}