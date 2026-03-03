#include "Player/BACharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
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
#include "BAParkourComponent.h"
#include "Net/UnrealNetwork.h"
#include "UI/UW_PlayerHUDWidget.h"
#include "GAS/AttributeSet/AmmoAttributeSet.h"
#include "GAS/BAGameplayTags.h"
#include "AbilitySystemComponent.h"
//#include "DrawDebugHelpers.h"//디버그 용 빨간 선
#include "Common/BAItemInterface.h"
#include "GAS/AttributeSet/HealthAttributeSet.h"
#include "Weapon/BaseRangedWeapon.h"
#include "Weapon/Data/WeaponDataAsset.h"
#include "Kismet/KismetSystemLibrary.h"
//건축
#include "Building/BuildManagerComponent.h"
#include "Components/SceneCaptureComponent2D.h"

// Sets default values
ABACharacter::ABACharacter()
{
	// Tick 설정
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	// 캐릭터 회전 설정
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;


	// 이동 컴포넌트 설정
	GetCharacterMovement()->bOrientRotationToMovement = false;
	//GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // 회전 속도
	GetCharacterMovement()->MaxWalkSpeed = UpdateMovementSpeed();
	GetCharacterMovement()->MaxWalkSpeedCrouched = CrouchSpeed;

	DetectedCapsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("DetectedCapsule"));
	DetectedCapsule->SetupAttachment(RootComponent);
	DetectedCapsule->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	DetectedCapsule->SetCollisionObjectType(ECollisionChannel::ECC_GameTraceChannel3);
	DetectedCapsule->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel7, ECollisionResponse::ECR_Overlap);	// Enemy Vision

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->bUsePawnControlRotation = true;
	// 카메라 생성 및 설정
	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	CameraComponent->bUsePawnControlRotation = false;
	CameraComponent->SetupAttachment(SpringArm);

	MotionWarpingComp = CreateDefaultSubobject<UMotionWarpingComponent>(TEXT("MotionWarping"));
	MotionWarpingComp->bAutoActivate = true;
	bIsTurning = false;
	TurnDelayTimer = 0.f;
	SpringArmZ = 0.f;

	//파쿠르
	ParkourComponent = CreateDefaultSubobject<UBAParkourComponent>(TEXT("ParkourComponent"));

	//GAS
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	HealthAttributeSet = CreateDefaultSubobject<UHealthAttributeSet>(TEXT("HealthSet"));
	AmmoAttributeSet = CreateDefaultSubobject<UAmmoAttributeSet>(TEXT("AmmoSet"));

	//Test
	//앉기 기능 활성화
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;

	bIsAiming = false;
	bIsRunning = false;

	BuildManager = CreateDefaultSubobject<UBuildManagerComponent>(TEXT("BuildManager"));

	SceneCaptureParent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SceneCaptureArm"));
	SceneCaptureParent->SetupAttachment(RootComponent);
	SceneCaptureParent->bUsePawnControlRotation = false;
	SceneCaptureParent->bInheritPitch = false;
	SceneCaptureParent->bInheritYaw = false;
	SceneCaptureParent->bInheritRoll = false;
	SceneCaptureParent->SetRelativeRotation(ScannerDefaultRotation);

	SceneCapture2D = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("SceneCapture2D"));
	SceneCapture2D->SetupAttachment(SceneCaptureParent);
	SceneCapture2D->CaptureSource = ESceneCaptureSource::SCS_FinalColorHDR;
	SceneCapture2D->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_UseShowOnlyList;
	SceneCapture2D->SetRelativeLocation(FVector(-DefaultScannerDistance, 0.0f, 0.0f));
	SceneCapture2D->ShowFlags.SetLighting(false);
	SceneCapture2D->ShowFlags.SetDynamicShadows(false);
	SceneCapture2D->ShowFlags.SetSkyLighting(false);
	SceneCapture2D->ShowFlags.SetAtmosphere(false);
	SceneCapture2D->ShowFlags.SetFog(false);
	SceneCapture2D->ShowFlags.SetBloom(false);
	SceneCapture2D->ShowFlags.SetEyeAdaptation(false);
	SceneCapture2D->ShowFlags.SetMotionBlur(false);
	SceneCapture2D->ShowFlags.SetAntiAliasing(false);
	SceneCapture2D->ShowFlags.SetTemporalAA(false);
	SceneCapture2D->ShowFlags.SetTonemapper(false);
	SceneCapture2D->ShowFlags.SetTranslucency(false);
	SceneCapture2D->ShowFlags.SetParticles(false);
	SceneCapture2D->ShowFlags.SetSkeletalMeshes(false);
	SceneCapture2D->ShowFlags.SetLandscape(false);
	SceneCapture2D->ShowFlags.SetGameplayDebug(false);
	SceneCapture2D->ShowFlags.SetCompositeDebugPrimitives(false);
	SceneCapture2D->ShowFlags.SetDebugAI(false);
	SceneCapture2D->ShowFlags.SetCollision(false);
	SceneCapture2D->ShowFlags.SetBounds(false);
	SceneCapture2D->ShowFlags.SetMaterials(true);
	SceneCapture2D->ShowFlags.SetStaticMeshes(true);
	SceneCapture2D->ShowFlags.SetPostProcessing(true);

	ArrowMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ArrowMesh"));
	ArrowMesh->SetupAttachment(RootComponent);
	ArrowMesh->SetVisibleInSceneCaptureOnly(true);
	ArrowMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ArrowMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	ArrowMesh->SetGenerateOverlapEvents(false);
	ArrowMesh->SetRenderCustomDepth(true);
	ArrowMesh->CustomDepthStencilValue = 1;
	SceneCapture2D->ShowOnlyComponents.Add(ArrowMesh);
}

// Called when the game starts or when spawned
void ABACharacter::BeginPlay()
{
	Super::BeginPlay();
	LastBodyYaw = GetMesh()->GetComponentRotation().Yaw;

	if (IsLocallyControlled() == true)
	{
		SceneCapture2D->PostProcessSettings.AddBlendable(M_PostProcessGroundScanner, 1.0f);
		SceneCapture2D->TextureTarget = RT_GroundScanner;
	}
}

void ABACharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (HasAuthority() && EquippedWeapon)
	{
		EquippedWeapon->Destroy();
	}

	Super::EndPlay(EndPlayReason);
}

// Called every frame
void ABACharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);


	float Speed = GetVelocity().Size2D();

	/*if (Speed > 1.f)
		bUseControllerRotationYaw = true;
	else
		bUseControllerRotationYaw = false;*/

	if (HasAuthority())
	{
		if (Controller)
		{
			ControlRot = Controller->GetControlRotation();
			SyncAimPitch = ControlRot.Pitch;
			SyncAimYaw = ControlRot.Yaw;
		}
	}
	RootYawOffset = UKismetMathLibrary::NormalizeAxis(GetControlRotation().Yaw - LastBodyYaw);

	IdleTurning(DeltaTime);

	if (IsLocallyControlled())
	{
		if (!FMath::IsNearlyZero(CurrentRecoilPitch))
		{
			float ApplyAmountPitnch = FMath::FInterpTo(0.f, CurrentRecoilPitch, DeltaTime, RecoilInterpSpeed);
			AddControllerPitchInput(-ApplyAmountPitnch);
			CurrentRecoilPitch -= ApplyAmountPitnch;
			if (FMath::IsNearlyZero(CurrentRecoilPitch))
			{
				CurrentRecoilPitch = 0.f;
			}
		}
		if (!FMath::IsNearlyZero(CurrentRecoilYaw))
		{
			float ApplyAmountYaw = FMath::FInterpTo(0.f, CurrentRecoilYaw, DeltaTime, RecoilInterpSpeed);
			AddControllerYawInput(ApplyAmountYaw);

			CurrentRecoilYaw -= ApplyAmountYaw;
			if (FMath::IsNearlyZero(CurrentRecoilYaw))
			{
				CurrentRecoilYaw = 0.f;
			}
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
			EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ABACharacter::JumpHandler);
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
			EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Started, this, &ABACharacter::StartAttack);
			EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Completed, this, &ABACharacter::StopAttack);
		}

		if (ReloadAction)
		{
			EnhancedInputComponent->BindAction(ReloadAction, ETriggerEvent::Started, this, &ABACharacter::Reload);
		}
		// 조준
		if (AimAction)
		{
			EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Started, this, &ABACharacter::AimStart);
			EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Completed, this, &ABACharacter::AimStop);
		}

		if (ADSAction)
		{
			EnhancedInputComponent->BindAction(ADSAction, ETriggerEvent::Completed, this, &ABACharacter::ADSStart);
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

		// 건설
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
		if (SelectCat1Action)
		{
			EnhancedInputComponent->BindAction(SelectCat1Action, ETriggerEvent::Started, this, &ABACharacter::OnSelectCat1);
		}
		if (SelectCat2Action)
		{
			EnhancedInputComponent->BindAction(SelectCat2Action, ETriggerEvent::Started, this, &ABACharacter::OnSelectCat2);
		}
		if (SelectCat3Action)
		{
			EnhancedInputComponent->BindAction(SelectCat3Action, ETriggerEvent::Started, this, &ABACharacter::OnSelectCat3);
		}
		if (CyclePrevAction)
		{
			EnhancedInputComponent->BindAction(CyclePrevAction, ETriggerEvent::Started, this, &ABACharacter::OnCyclePrev);
		}
		if (CycleNextAction)
		{
			EnhancedInputComponent->BindAction(CycleNextAction, ETriggerEvent::Started, this, &ABACharacter::OnCycleNext);
		}

		// 지하
		if (GroundScannerAction)
		{
			EnhancedInputComponent->BindAction(GroundScannerAction, ETriggerEvent::Started, this, &ABACharacter::SwitchGroundScanner);
		}
	}
}

void ABACharacter::OnRep_Controller()
{
	Super::OnRep_Controller();
	if (IsLocallyControlled())
	{
		if (AbilitySystemComponent)
		{
			AbilitySystemComponent->InitAbilityActorInfo(this, this);

			for (const TSubclassOf<UGameplayAbility>& AbilityClass : DefaultAbility)
			{
				if (AbilityClass)
				{
					FGameplayAbilitySpec Spec(AbilityClass, 1, -1, this);
					AbilitySystemComponent->GiveAbility(Spec);
				}
			}

			AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(HealthAttributeSet->GetHealthAttribute())
				.AddUObject(this, &ABACharacter::OnHealthChangedCallback);

			AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AmmoAttributeSet->GetCurrentAmmoAttribute())
				.AddUObject(this, &ABACharacter::OnAmmoChangedCallback);

			AbilitySystemComponent->RegisterGameplayTagEvent(
				TAG_State_Combat_Dead,
				EGameplayTagEventType::NewOrRemoved
			).AddUObject(this, &ABACharacter::HandleRespawnUI);
		}
	}
}
void ABACharacter::SpringArmRot(bool check)
{
	SpringArm->bUsePawnControlRotation = check;
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

	StopMontage();

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
	Server_SetRunning(true);
}

void ABACharacter::StopRunning(const FInputActionValue& Value)
{
	bIsRunning = false;
	GetCharacterMovement()->MaxWalkSpeed = UpdateMovementSpeed();
	Server_SetRunning(false);
}

void ABACharacter::Server_SetRunning_Implementation(bool bNewIsRunning)
{
	bIsRunning = bNewIsRunning;
	GetCharacterMovement()->MaxWalkSpeed = UpdateMovementSpeed();
}

void ABACharacter::CrouchInput(const FInputActionValue& Value)
{
	if (!bIsCrouched)
	{
		Crouch(false);
		if (bIsRunning)
			bIsRunning = false;
		SpringArmZ = -30.f;

		StopMontage();
	}
	else
	{
		UnCrouch(false);
		SpringArmZ = 0.f;

		StopMontage();
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

void ABACharacter::StartAttack(const FInputActionValue& Value)
{
	if (!AbilitySystemComponent) return;
	if (!EquippedWeapon) return;

	FGameplayTagContainer Tag;

	UWeaponDataAsset* WeaponData = EquippedWeapon->GetWeaponData();
	if (!WeaponData) return;
	Tag.AddTag(WeaponData->WeaponTag);

	AbilitySystemComponent->TryActivateAbilitiesByTag(Tag);
}

void ABACharacter::Reload(const FInputActionValue& Value)
{
	if (!AbilitySystemComponent) return;
	if (bIsADS)
	{
		ADSStart(1.f);
	}
	FGameplayTagContainer Tag;
	Tag.AddTag(TAG_Ability_Active_Reload);

	AbilitySystemComponent->TryActivateAbilitiesByTag(Tag);
}

void ABACharacter::StopAttack(const FInputActionValue& Value)
{
	if (!AbilitySystemComponent) return;
	if (!EquippedWeapon || !EquippedWeapon->bAutoActive) return;

	FGameplayTagContainer CancelTags;
	CancelTags.AddTag(TAG_Ability_Active);

	AbilitySystemComponent->CancelAbilities(&CancelTags);
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
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AmmoAttributeSet->GetCurrentAmmoAttribute())
			.AddUObject(this, &ABACharacter::OnAmmoChangedCallback);

		if (IsLocallyControlled())
		{
			AbilitySystemComponent->RegisterGameplayTagEvent(
				TAG_State_Combat_Dead,
				EGameplayTagEventType::NewOrRemoved
			).AddUObject(this, &ABACharacter::HandleRespawnUI);
		}
	}

	if (DefaultWeaponClass)
	{
		Server_EquipWeapon(DefaultWeaponClass);
	}
}

void ABACharacter::OnHealthChangedCallback(const FOnAttributeChangeData& Data) const
{
	OnHealthChanged.Broadcast(Data.NewValue, HealthAttributeSet->GetMaxHealth());
}

void ABACharacter::Server_EquipWeapon_Implementation(TSubclassOf<ABaseWeapon> WeaponClass)
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

FVector ABACharacter::GetFireStartLocation_Implementation() const
{
	if (ABaseRangedWeapon* Weapon = Cast<ABaseRangedWeapon>(EquippedWeapon))
	{
		USkeletalMeshComponent* WeaponMesh = Weapon->GetWeaponMesh();
		if (WeaponMesh && WeaponMesh->DoesSocketExist(Weapon->GetMuzzleSocketName()))
		{
			return WeaponMesh->GetSocketLocation(Weapon->GetMuzzleSocketName());
		}
	}

	return GetActorLocation();
}

FVector ABACharacter::GetFireDirection_Implementation() const
{
	if (ABaseRangedWeapon* Weapon = Cast<ABaseRangedWeapon>(EquippedWeapon))
	{
		USkeletalMeshComponent* WeaponMesh = Weapon->GetWeaponMesh();
		if (WeaponMesh && WeaponMesh->DoesSocketExist(Weapon->GetMuzzleSocketName()))
		{
			if (bIsADS)
			{
				return WeaponMesh->GetSocketRotation(Weapon->GetMuzzleSocketName()).Vector();
			}

			FVector WeaponSocketLocation = WeaponMesh->GetSocketLocation(Weapon->GetMuzzleSocketName());
			FVector ViewLoc;
			FRotator ViewRot;
			GetActorEyesViewPoint(ViewLoc, ViewRot);

			FVector AimEnd = ViewLoc + ViewRot.Vector() * 1000000.f;

			return (AimEnd - WeaponSocketLocation).GetSafeNormal();
		}
	}

	return GetActorRotation().Vector();
}

void ABACharacter::OnRep_bIsFiring()
{
	UBAAnimInstance* Anim = Cast<UBAAnimInstance>(GetMesh()->GetAnimInstance());
	if (Anim)
	{
		Anim->SetIsFiring(bIsFiring);
	}
}

void ABACharacter::SetbIsFiring(bool InIsFiring)
{
	bIsFiring = InIsFiring;
	UBAAnimInstance* Anim = Cast<UBAAnimInstance>(GetMesh()->GetAnimInstance());
	if (Anim)
	{
		Anim->SetIsFiring(bIsFiring);
	}
}

void ABACharacter::OnAmmoChangedCallback(const FOnAttributeChangeData& Data) const
{
	OnAmmoChanged.Broadcast(Data.NewValue, AmmoAttributeSet->GetMaxAmmo());
}

void ABACharacter::SetRecoil(float InPitch, float InYaw)
{
	if (!IsValid(this)) return;
	CurrentRecoilPitch = InPitch;
	CurrentRecoilYaw = InYaw;
}

void ABACharacter::HandleRespawnUI(FGameplayTag Tag, int32 NewCount)
{
	ABAPlayerController* PC = Cast<ABAPlayerController>(GetController());
	if (NewCount >= 1)
	{
		PC->StartRespawnBar(RespawnTime);
	}
	else if (NewCount == 0)
	{
		PC->StopRespawnBar();
	}
}

//조준 시작
void ABACharacter::AimStart(const FInputActionValue& Value)
{
	if (!AbilitySystemComponent->HasMatchingGameplayTag(TAG_Weapon_Equipped_Ranged)) return;
	bIsAiming = true;

	GetCharacterMovement()->MaxWalkSpeed = UpdateMovementSpeed();
	//bUseControllerRotationYaw = true;
	Server_SetAiming(true);
}

//조준 끝
void ABACharacter::AimStop(const FInputActionValue& Value)
{
	if (!bIsAiming) return;
	bIsAiming = false;

	GetCharacterMovement()->MaxWalkSpeed = UpdateMovementSpeed();
	//bUseControllerRotationYaw = false;
	Server_SetAiming(false);
}

void ABACharacter::ADSStart(const FInputActionValue& Value)
{
	if (!AbilitySystemComponent->HasMatchingGameplayTag(TAG_Weapon_Equipped_Ranged)) return;
	bIsADS = !bIsADS;

	ABAPlayerController* PC = Cast<ABAPlayerController>(GetController());
	if (PC)
	{
		if (bIsADS)
		{
			PC->StartADSUI();
			AimStart(1.f);

			SavedSpringArmTransform = SpringArm->GetRelativeTransform();

			SpringArm->AttachToComponent(EquippedWeapon->GetWeaponMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, "ADS_Sight");
			SpringArm->TargetArmLength = 0.f;
		}
		else
		{
			PC->StopADSUI();
			AimStop(1.f);

			SpringArm->AttachToComponent(RootComponent, FAttachmentTransformRules::SnapToTargetIncludingScale);
			SpringArm->SetRelativeTransform(SavedSpringArmTransform);
			SpringArm->TargetArmLength = 223.f;
		}
	}
}


void ABACharacter::Server_SetAiming_Implementation(bool bNewIsAiming)
{
	bIsAiming = bNewIsAiming;

	GetCharacterMovement()->MaxWalkSpeed = UpdateMovementSpeed();
}

void ABACharacter::Interaction(const FInputActionValue& Value)
{
	FVector Start = CameraComponent->GetComponentLocation();
	FVector Forward = CameraComponent->GetForwardVector();
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
			bool bHasInterface = HitActor->GetClass()->ImplementsInterface(UBAItemInterface::StaticClass());
			if (bHasInterface)
			{
				UE_LOG(LogTemp, Warning, TEXT("라인트레이스 상호작용: %s"), *HitActor->GetName());
				IBAItemInterface::Execute_Use(HitActor, this);
			}
		}
	}
}

void ABACharacter::EnterBuildMode(const FInputActionValue& Value)
{
	UE_LOG(LogTemp, Warning, TEXT(" [1단계] 캐릭터: B키 입력 감지 성공!"));
	if (!BuildManager->IsBuildMode())
	{
		BuildManager->EnterBuildMode();
	}
	else
	{
		BuildManager->ExitBuildMode();
	}
	ABAPlayerController* PC = Cast<ABAPlayerController>(GetController());
	if (PC)
	{
		PC->SwitchingMode();
	}
}

void ABACharacter::ExitBuildMode(const FInputActionValue& Value)
{
	if (!BuildManager->IsBuildMode())
	{
		return;
	}

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

void ABACharacter::OnSelectCat1(const FInputActionValue& Value)
{
	BuildManager->OnSelectCat1();
}

void ABACharacter::OnSelectCat2(const FInputActionValue& Value)
{
	BuildManager->OnSelectCat2();
}

void ABACharacter::OnSelectCat3(const FInputActionValue& Value)
{
	BuildManager->OnSelectCat3();
}

void ABACharacter::OnCyclePrev(const FInputActionValue& Value)
{
	BuildManager->OnCyclePrev();
}

void ABACharacter::OnCycleNext(const FInputActionValue& Value)
{
	BuildManager->OnCycleNext();
}

void ABACharacter::StartSwitchWeapon(const FInputActionValue& Value)
{
	int32 Index = (int32)Value.Get<float>() - 1;
	if (!OwnedEquipment.IsValidIndex(Index)) return;

	if (bIsADS)
	{
		ADSStart(1.f);
	}

	Server_EquipWeapon(OwnedEquipment[Index]);
}

void ABACharacter::JumpHandler(const FInputActionValue& Value)
{
	bool bParkourStarted = false;

	if (ParkourComponent)
	{
		bParkourStarted = ParkourComponent->AttemptParkour();
	}

	if (!bParkourStarted)
	{
		Jump();
	}
}
void ABACharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABACharacter, bIsAiming);
	DOREPLIFETIME(ABACharacter, bIsRunning);
	DOREPLIFETIME(ABACharacter, SyncAimYaw);
	DOREPLIFETIME(ABACharacter, SyncAimPitch);
	DOREPLIFETIME(ABACharacter, EquippedWeapon);
}

void ABACharacter::Multicast_PlayTurnMontage_Implementation(UAnimMontage* MontageToPlay, FTransform TargetTransform)
{
	if (MotionWarpingComp)
	{
		MotionWarpingComp->AddOrUpdateWarpTargetFromTransform(FName("TurnTarget"), TargetTransform);
	}

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && MontageToPlay)
	{
		AnimInstance->Montage_Play(MontageToPlay);

		FOnMontageEnded EndDelegate;
		EndDelegate.BindUObject(this, &ABACharacter::OnTurnMontageEnded);
		AnimInstance->Montage_SetEndDelegate(EndDelegate, MontageToPlay);
	}

	bIsTurning = true;
	CurrentTurnMontage = MontageToPlay;
}
void ABACharacter::ServerRPC_StopTurnMontage_Implementation()
{
	Multicast_StopTurnMontage();
}
void ABACharacter::Multicast_StopTurnMontage_Implementation()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && CurrentTurnMontage)
	{
		AnimInstance->Montage_Stop(0.5f, CurrentTurnMontage);
	}
	SetTurnStatus();
}

void ABACharacter::OnTurnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (CurrentTurnMontage == Montage)
	{
		SetTurnStatus();
	}
}
void ABACharacter::IdleTurning(float DeltaTime)
{
	/*if (!HasAuthority()) return;
	if (!bIsTurning)
	{
		if (FMath::Abs(RootYawOffset) > 90.f)
		{
			TurnDelayTimer += DeltaTime;
			if (TurnDelayTimer > 0.1f)
			{
				UE_LOG(LogTemp, Warning, TEXT("90도 넘음"));
				bIsTurning = true;

				bool bRight = (RootYawOffset < 0);

				if (FMath::Abs(RootYawOffset) > 135.f)
				{
					TurnType = bRight ? ETurnType::Right180 : ETurnType::Left180;
					CurrentTurnSpeed = 15.f;
				}
				else if (FMath::Abs(RootYawOffset) > 90.f)
				{
					TurnType = bRight ? ETurnType::Right90 : ETurnType::Left90;
					CurrentTurnSpeed = 10.f;
				}
				CurrentTurnMontage = nullptr;
				if (bIsCrouched)
				{
					switch (TurnType)
					{
					case ETurnType::Left90:
						CurrentTurnMontage = CrouchTurnLeft90Montage;
						break;
					case ETurnType::Right90:
						CurrentTurnMontage = CrouchTurnRight90Montage;
						break;
					case ETurnType::Left180:
						CurrentTurnMontage = CrouchTurnLeft180Montage;
						break;
					case ETurnType::Right180:
						CurrentTurnMontage = CrouchTurnRight180Montage;
						break;
					default:
						return;
					}
				}
				else
				{
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
				}
				if (CurrentTurnMontage && MotionWarpingComp)
				{
					FRotator GoalRot = FRotator(0.f, GetControlRotation().Yaw, 0.f);

					FTransform TargetTransform(GoalRot, GetActorLocation());
					Multicast_PlayTurnMontage(CurrentTurnMontage, TargetTransform);
				}
			}
		}
		else
		{
			TurnDelayTimer = 0.f;
		}
	}*/
}

void ABACharacter::SetTurnStatus()
{
	bIsTurning = false;
	TurnType = ETurnType::None;
	CurrentTurnMontage = nullptr;
	TurnDelayTimer = 0.f;
	LastBodyYaw = GetActorRotation().Yaw;
	RootYawOffset = 0.0f;
}

void ABACharacter::StopMontage()
{
	if (bIsTurning)
	{
		bIsTurning = false;
		TurnDelayTimer = 0.f;

		if (HasAuthority())
		{
			Multicast_StopTurnMontage();
		}
		else
		{
			ServerRPC_StopTurnMontage();
		}
	}
}

void ABACharacter::RotateScannerParent(const FVector2D& Input)
{
	if (IsValid(SceneCaptureParent) == false)
		return;

	FRotator CurrentRot = SceneCaptureParent->GetRelativeRotation();

	float NewYaw = CurrentRot.Yaw + Input.X * ScannerRotateMultiplier;
	float NewPitch = CurrentRot.Pitch - Input.Y * ScannerRotateMultiplier;
	NewPitch = FMath::Clamp(NewPitch, -80.f, 80.f);
	SceneCaptureParent->SetRelativeRotation(FRotator(NewPitch, NewYaw, CurrentRot.Roll));
}

void ABACharacter::ChangeScannerDistance(float Input)
{
	float ScannerDistance = FMath::Clamp(-SceneCapture2D->GetRelativeLocation().X + Input * ScannerZoomMultiplier, MinScannerDistance, MaxScannerDistance);

	SceneCapture2D->SetRelativeLocation(FVector(-ScannerDistance, 0.0f, 0.0f));
}

void ABACharacter::SwitchGroundScanner()
{
	FRotator DefaultRotation = ScannerDefaultRotation + FRotator(0.0f, GetActorRotation().Yaw, 0.0f);
	SceneCaptureParent->SetRelativeRotation(DefaultRotation);
	SceneCapture2D->SetRelativeLocation(FVector(-DefaultScannerDistance, 0.0f, 0.0f));

	ABAPlayerController* PC = Cast<ABAPlayerController>(GetController());
	if (PC)
	{
		PC->SwitchGroundScanner();
	}
}
