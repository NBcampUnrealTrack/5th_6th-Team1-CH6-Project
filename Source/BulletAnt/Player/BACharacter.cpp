#include "Player/BACharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"
#include "BAPlayerController.h"
#include "MotionWarpingComponent.h"
#include "BAAnimInstance.h"
//#include "DrawDebugHelpers.h"//디버그 용 빨간 선
#include "Components/CapsuleComponent.h"
#include "Common/BAItemInterface.h"
#include "AbilitySystemComponent.h"
#include "GAS/AttributeSet/HealthAttributeSet.h"
#include "Weapon/BaseRangedWeapon.h"
#include "Weapon/Data/WeaponDataAsset.h"
#include "Kismet/KismetSystemLibrary.h"
//건축
#include "Building/BuildManagerComponent.h"

// Sets default values
ABACharacter::ABACharacter()
{
	// Tick 설정
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	// 캐릭터 회전 설정
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;


    // 이동 컴포넌트 설정
    GetCharacterMovement()->bOrientRotationToMovement = false;
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // 회전 속도
    GetCharacterMovement()->MaxWalkSpeed = UpdateMovementSpeed();
    GetCharacterMovement()->MaxWalkSpeedCrouched = CrouchSpeed;

	GetMesh()->SetOwnerNoSee(true);
	GetMesh()->bCastHiddenShadow = true;

	// 카메라 생성 및 설정
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FirstPersonCameraComponent->bUsePawnControlRotation = true;
	FirstPersonCameraComponent->SetupAttachment(GetMesh(), TEXT("HEAD"));

	//1인칭
	FPSMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FPSMesh"));
	FPSMesh->SetupAttachment(FirstPersonCameraComponent);
	FPSMesh->SetOnlyOwnerSee(true);
	FPSMesh->SetCastShadow(false);
	FPSMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	MotionWarpingComp = CreateDefaultSubobject<UMotionWarpingComponent>(TEXT("MotionWarping"));
	MotionWarpingComp->bAutoActivate = true;
	bIsTurning = false;
	TurnDelayTimer = 0.f;

	//GAS
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	HealthAttributeSet = CreateDefaultSubobject<UHealthAttributeSet>(TEXT("HealthSet"));

	//Test
	//앉기 기능 활성화
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;

	bIsAiming = false;
	bIsRunning = false;
	bIsClimbing = false;
	ClimbDuration = 1.f;

	BuildManager = CreateDefaultSubobject<UBuildManagerComponent>(TEXT("BuildManager"));
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

	if (HasAuthority())
	{
		if(Controller)
		{
			ControlRot = Controller->GetControlRotation();
			SyncAimPitch = ControlRot.Pitch;
			SyncAimYaw = ControlRot.Yaw;
		}
	}
    DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(ControlRot, GetActorRotation());

	if (!bIsAiming)
		IdleCrouchTurning(DeltaTime);
	else
		AimTurning(DeltaTime);
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

		if (SwitchAction)
		{
			EnhancedInputComponent->BindAction(SwitchAction, ETriggerEvent::Started, this, &ABACharacter::StartSwitchWeapon);
		}

		if (EnterBuildModeAction)
		{
			EnhancedInputComponent->BindAction(EnterBuildModeAction, ETriggerEvent::Started, this, &ABACharacter::EnterBuildMode);
		}
		if (ExitBuildModeAction)
		{
			EnhancedInputComponent->BindAction(ExitBuildModeAction, ETriggerEvent::Started, this, &ABACharacter::ExitBuildMode);
		}
		if (PlaceBuildingAction)
		{
			EnhancedInputComponent->BindAction(PlaceBuildingAction, ETriggerEvent::Started, this, &ABACharacter::PlaceBuilding);
		}
		if (RotateBuildingAction)
		{
			EnhancedInputComponent->BindAction(RotateBuildingAction, ETriggerEvent::Started, this, &ABACharacter::RotateBuilding);
		}
		if (ToggleSnapModeAction)
		{
			EnhancedInputComponent->BindAction(ToggleSnapModeAction, ETriggerEvent::Started, this, &ABACharacter::ToggleSnapMode);
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

	if (bIsTurning)
	{
		UE_LOG(LogTemp, Warning, TEXT("움직였다"));
		bIsTurning = false;
		TurnType = ETurnType::None;
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (AnimInstance)
		{
			AnimInstance->StopAllMontages(0.1f);
		}
		TurnDelayTimer = 0.f;
	}

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
	return EquippedWeapon ? EquippedWeapon->GetWeaponData() : nullptr;
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

	FGameplayTagContainer Tag;

	UWeaponDataAsset* WeaponData = EquippedWeapon->GetWeaponData();
	if (!WeaponData) return;
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
		/*EquippedWeapon->UnequipWeapon(AbilitySystemComponent);*/
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
    FVector Start = FirstPersonCameraComponent->GetComponentLocation();
    FVector Forward = FirstPersonCameraComponent->GetForwardVector();
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

void ABACharacter::EnterBuildMode(const FInputActionValue& Value)
{
	UE_LOG(LogTemp, Warning, TEXT(" [1단계] 캐릭터: B키 입력 감지 성공!"));
	BuildManager->EnterBuildMode();
	ABAPlayerController* PC = Cast<ABAPlayerController>(GetController());
	if (PC)
	{
		PC->SwitchingMode();
	}
}

void ABACharacter::ExitBuildMode(const FInputActionValue& Value)
{
	BuildManager->ExitBuildMode();
	ABAPlayerController* PC = Cast<ABAPlayerController>(GetController());
	if (PC)
	{
		PC->SwitchingMode();
	}
}

void ABACharacter::PlaceBuilding(const FInputActionValue& Value)
{
	BuildManager->TryPlace();
}

void ABACharacter::RotateBuilding(const FInputActionValue& Value)
{
	BuildManager->RotatePreviewByWheel(Value);
}

void ABACharacter::ToggleSnapMode(const FInputActionValue& Value)
{
	BuildManager->ToggleSnapMode();
}


void ABACharacter::StartClimb(FVector TargetLocation)
{
	if (bIsClimbing) return;

	GetCharacterMovement()->SetMovementMode(MOVE_Flying);
	// 벽과 충돌을 꺼야 할 경우
	//GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	bIsClimbing = true;
	ClimbStartLocation = GetActorLocation();
	ClimbEndLocation = TargetLocation;
	ClimbTimer = 0.f;
}

void ABACharacter::EndClimb()
{
	bIsClimbing = false;

	GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	// 벽과 충돌을 꺼야 할 경우
	//GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
}
void ABACharacter::StartSwitchWeapon(const FInputActionValue& Value)
{
	EquipWeapon(OwnedEquipment[(int32)Value.Get<float>()-1]);
}

void ABACharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABACharacter, bIsAiming);
	DOREPLIFETIME(ABACharacter, bIsRunning);
	DOREPLIFETIME(ABACharacter, bIsClimbing);
	DOREPLIFETIME(ABACharacter, SyncAimYaw);
	DOREPLIFETIME(ABACharacter, SyncAimPitch);
}

void ABACharacter::IdleCrouchTurning(float DeltaTime)
{
	if (!bIsTurning)
	{
		if (FMath::Abs(DeltaRot.Yaw) > 90.f)
		{
			TurnDelayTimer += DeltaTime;
			if (TurnDelayTimer > 0.5f)
			{
				UE_LOG(LogTemp, Warning, TEXT("90도 넘음"));
				bIsTurning = true;

				bool bRight = (DeltaRot.Yaw > 0);

				if (FMath::Abs(DeltaRot.Yaw) > 135.f)
				{
					TurnType = bRight ? ETurnType::Right180 : ETurnType::Left180;
					CurrentTurnSpeed = 15.f;
				}
				else if (FMath::Abs(DeltaRot.Yaw) > 90.f)
				{
					TurnType = bRight ? ETurnType::Right90 : ETurnType::Left90;
					CurrentTurnSpeed = 10.f;
				}
				UAnimMontage* CurrentTurnMontage = nullptr;
				switch (TurnType)
				{
				case ETurnType::Left90:
					CurrentTurnMontage = TurnLeft90Montage;
					break;
				case ETurnType::Right90:
					CurrentTurnMontage = TurnRight90Montage;
					break;
				case ETurnType::Left180:
					CurrentTurnMontage = TurnLeft180Montage;
					break;
				case ETurnType::Right180:
					CurrentTurnMontage = TurnRight180Montage;
					break;
				default:
					return;
				}
				if (CurrentTurnMontage && MotionWarpingComp)
				{
					FRotator GoalRot = FRotator(0.f, GetControlRotation().Yaw, 0.f);

					FTransform TargetTransform(GoalRot, GetActorLocation());
					MotionWarpingComp->AddOrUpdateWarpTargetFromTransform(FName("TurnTarget"), TargetTransform);
					PlayAnimMontage(CurrentTurnMontage);
				}
			}
		}
	}
}

void ABACharacter::AimTurning(float DeltaTime)
{

	if (!bIsTurning)
	{
		if (FMath::Abs(DeltaRot.Yaw) > 45.f)
		{
			bIsTurning = true;

			bool bRight = (DeltaRot.Yaw > 0);

			if (FMath::Abs(DeltaRot.Yaw) > 135.f)
			{
				TurnType = bRight ? ETurnType::Right180 : ETurnType::Left180;
				CurrentTurnSpeed = 15.f;
			}
			else if (FMath::Abs(DeltaRot.Yaw) > 45.f)
			{
				TurnType = bRight ? ETurnType::Right90 : ETurnType::Left90;
				CurrentTurnSpeed = 15.f;
			}
		}
	}
	if (bIsTurning)
	{
		if (FMath::Abs(DeltaRot.Yaw) < 5.f)
		{
			bIsTurning = false;
			TurnType = ETurnType::None;
		}
		else
		{
			if (bIsAiming)
			{
				if (FMath::Abs(DeltaRot.Yaw) > AimTurn)
				{
					FRotator NewRot = FMath::RInterpTo(GetActorRotation(), ControlRot, DeltaTime, 20.f);
					SetActorRotation(FRotator(0.f, SyncAimYaw, 0.f));
				}
			}
			else
			{
				if (FMath::Abs(DeltaRot.Yaw) > IdleTurn)
				{
					FRotator NewRot = FMath::RInterpTo(GetActorRotation(), ControlRot, DeltaTime, 20.f);
					SetActorRotation(FRotator(0.f, NewRot.Yaw, 0.f));
				}
			}

			if (bIsClimbing)
			{
				ClimbTimer += DeltaTime;

				float Alpha = ClimbTimer / ClimbDuration;
				FVector NewLocation = FMath::Lerp(ClimbStartLocation, ClimbEndLocation, Alpha);

				SetActorLocation(NewLocation);

				if (Alpha >= 1.f)
					EndClimb();
			}
		}
	}
}