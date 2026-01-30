// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/BACharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"
//#include "DrawDebugHelpers.h"//디버그 용 빨간 선
#include "Components/CapsuleComponent.h"
#include "Common/BAItemInterface.h"
#include "AbilitySystemComponent.h"
#include "GAS/AttributeSet/HealthAttributeSet.h"
#include "Weapon/BaseRangedWeapon.h"
#include "Weapon/Data/WeaponDataAsset.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Player/ParkourComponent.h"

// Sets default values
ABACharacter::ABACharacter()
{
	// Tick 설정
	PrimaryActorTick.bCanEverTick = true;

	// 캐릭터 회전 설정
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;


    // 이동 컴포넌트 설정
    GetCharacterMovement()->bOrientRotationToMovement = false;
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // 회전 속도
    GetCharacterMovement()->MaxWalkSpeed = UpdateMovementSpeed();
    GetCharacterMovement()->MaxWalkSpeedCrouched = CrouchSpeed;

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

	HealthAttributeSet = CreateDefaultSubobject<UHealthAttributeSet>(TEXT("HealthSet"));

	// 파쿠르 컴포넌트
	ParkourComponent = CreateDefaultSubobject<UParkourComponent>(TEXT("ParkourComponent"));

	//앉기 기능 활성화
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;

	bIsAiming = false;
	bIsRunning = false;
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
    float Speed = GetVelocity().Size2D();
    if (Speed > 1.f)
        bUseControllerRotationYaw = true;
    else
        bUseControllerRotationYaw = false;

    FRotator ControlRot = GetControlRotation();
    FRotator ActorRot = GetActorRotation();
    FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(ControlRot, ActorRot);
    if(bIsAiming)
    {
        if (FMath::Abs(DeltaRot.Yaw) > AimTurn)
        {
            SetActorRotation(FRotator(0.f, ControlRot.Yaw, 0.f));
        }
    }
    else
    {
        if (FMath::Abs(DeltaRot.Yaw) > IdleTurn)
        {
            SetActorRotation(FRotator(0.f, ControlRot.Yaw, 0.f));
        }
    }

}

// 입력 바인딩
void ABACharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {

		// 점프
		if (JumpAction)
		{
			EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
			EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
		}

		// 이동
		if (MoveAction)
		{
			EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ABACharacter::Move);
		}
		// 달리기
		if (RunAction)
		{
			EnhancedInputComponent->BindAction(RunAction, ETriggerEvent::Started, this, &ABACharacter::StartRunning);
			EnhancedInputComponent->BindAction(RunAction, ETriggerEvent::Completed, this, &ABACharacter::StopRunning);
		}
		// 앉기
		if (CrouchAction)
		{
			EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Started, this, &ABACharacter::CrouchInput);
		}
		// 시선 처리 (마우스)
		if (LookAction)
		{
			EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ABACharacter::Look);
		}

		if (AttackAction)
		{
			EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Started, this, &ABACharacter::Attack);
		}
		// 조준
		if (AimAction)
		{
			EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Started, this, &ABACharacter::AimStart);
			EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Completed, this, &ABACharacter::AimStop);
		}

		//상호작용
		if (InteractionAction)
		{
			EnhancedInputComponent->BindAction(InteractionAction, ETriggerEvent::Started, this, &ABACharacter::Interaction);
		}
	}
}

//상태에 따른 이동속도
float ABACharacter::UpdateMovementSpeed()
{
    float NewSpeed = WalkSpeed;

    if (bIsAiming)
        NewSpeed = AimSpeed;
    else if (bIsRunning)
        NewSpeed = RunningSpeed;

    return NewSpeed;
}

// 이동 함수 구현
void ABACharacter::Move(const FInputActionValue& Value)
{
    if (!Controller) return;

    FVector2D MovementVector = Value.Get<FVector2D>();
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

UDataAsset* ABACharacter::GetDataAsset() const
{
	if (!EquippedWeapon)
	{
		return nullptr;
	}
	return EquippedWeapon->GetWeaponData();
}

void ABACharacter::StartRunning(const FInputActionValue& Value)
{
    bIsRunning = true;
    GetCharacterMovement()->MaxWalkSpeed = UpdateMovementSpeed();
}

void ABACharacter::StopRunning(const FInputActionValue& Value)
{
    bIsRunning = false;
    GetCharacterMovement()->MaxWalkSpeed = UpdateMovementSpeed();
}

void ABACharacter::CrouchInput(const FInputActionValue& Value)
{
    if(!bIsCrouched)
    {
        Crouch(false);
        if(bIsRunning)
            bIsRunning = false;
    }
    else
    {
        UnCrouch(false);
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

FVector ABACharacter::GetFireStartLocation() const
{
	if (ABaseRangedWeapon* Weapon = Cast<ABaseRangedWeapon>(EquippedWeapon))
	{
		USkeletalMeshComponent* WeaponMesh = Weapon->GetWeaponMesh();
		if (WeaponMesh && WeaponMesh->DoesSocketExist(Weapon->MuzzleSocketName))
		{
			return WeaponMesh->GetSocketLocation(Weapon->MuzzleSocketName);
		}
	}

	return GetActorLocation();
}

FVector ABACharacter::GetFireDirection() const
{
	if (ABaseRangedWeapon* Weapon = Cast<ABaseRangedWeapon>(EquippedWeapon))
	{
		USkeletalMeshComponent* WeaponMesh = Weapon->GetWeaponMesh();
		if (WeaponMesh && WeaponMesh->DoesSocketExist(Weapon->MuzzleSocketName))
		{
			return WeaponMesh->GetSocketRotation(Weapon->MuzzleSocketName).Vector();
		}
	}

	return GetActorRotation().Vector();
}


//조준 시작
void ABACharacter::AimStart(const FInputActionValue& Value)
{
	bIsAiming = true;

    GetCharacterMovement()->MaxWalkSpeed = UpdateMovementSpeed();
}

//조준 끝
void ABACharacter::AimStop(const FInputActionValue& Value)
{
	bIsAiming = false;

    GetCharacterMovement()->MaxWalkSpeed = UpdateMovementSpeed();
}

void ABACharacter::Interaction(const FInputActionValue& Value)
{
    FVector Start = FollowCamera->GetComponentLocation();
    FVector Forward = FollowCamera->GetForwardVector();
    FVector End = Start + (Forward * LineTraceRange);

    FHitResult HitResult;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);

    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        Start,
        End,
        ECC_Visibility,
        Params
    );

    if (bHit)
    {
        AActor* HitActor = HitResult.GetActor();

        if (HitActor)
        {
            UE_LOG(LogTemp, Warning, TEXT("라인트레이스 상호작용: %s"), *HitActor->GetName());
            IBAItemInterface::Execute_Use(HitActor, this);
        }
    }
}

