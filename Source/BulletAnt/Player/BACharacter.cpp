// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/BACharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "AbilitySystemComponent.h"
#include "GAS/AttributeSet/HealthAttributeSet.h"
#include "Weapon/Rifle/WeaponRifle.h"

// Sets default values
ABACharacter::ABACharacter()
{
    // Tick 설정
    PrimaryActorTick.bCanEverTick = true;

    // 회전 설정: 캐릭터는 컨트롤러 회전을 바로 따라가지 않음 (카메라만 따라감)
    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;

    // 이동 컴포넌트 설정: 이동 방향으로 캐릭터가 자동으로 회전하도록 함
    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // 회전 속도

    // 스프링 암 생성 및 설정
    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->TargetArmLength = 400.0f;
    CameraBoom->bUsePawnControlRotation = true;

    // 카메라 생성 및 설정
    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
    FollowCamera->bUsePawnControlRotation = false;
    
    //GAS
    AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
    AbilitySystemComponent->SetIsReplicated(true);
    AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

    //Test
}

// Called when the game starts or when spawned
void ABACharacter::BeginPlay()
{
    Super::BeginPlay();
}

// Called every frame
void ABACharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// 입력 바인딩 (여기가 핵심!)
void ABACharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {

        // 점프 (Character 기본 제공 함수 사용)
        if(JumpAction)
        {
            EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
            EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
        }

        // 이동
        if(MoveAction)
        {
            EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ABACharacter::Move);
        }

        // 시선 처리 (마우스)
        if(LookAction)
        {
            EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ABACharacter::Look);
        }

        if (AttackAction)
        {
            EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Started, this, &ABACharacter::Attack);
        }
    }
}

// 이동 함수 구현
void ABACharacter::Move(const FInputActionValue& Value)
{
    FVector2D MovementVector = Value.Get<FVector2D>();
    if (Controller != nullptr)
    {
        // 컨트롤러가 보고 있는 방향(Yaw)을 알아냄
        const FRotator Rotation = Controller->GetControlRotation();
        const FRotator YawRotation(0, Rotation.Yaw, 0);

        // 전방 방향 (W/S) 계산
        const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

        // 우측 방향 (A/D) 계산
        const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

        // 이동 적용
        AddMovementInput(ForwardDirection, MovementVector.Y);
        AddMovementInput(RightDirection, MovementVector.X);
    }
}

// 시선 처리 함수 구현
void ABACharacter::Look(const FInputActionValue& Value)
{
    FVector2D LookAxisVector = Value.Get<FVector2D>();

    if (Controller != nullptr)
    {
        AddControllerYawInput(LookAxisVector.X);
        AddControllerPitchInput(LookAxisVector.Y);
    }
}

void ABACharacter::Attack(const FInputActionValue& Value)
{
    if (!AbilitySystemComponent) return;

    UE_LOG(LogTemp, Error, TEXT("Attack"));

    FGameplayTagContainer Tag;
    Tag.AddTag(FGameplayTag::RequestGameplayTag(TEXT("Ability.Weapon.Fire")));

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
