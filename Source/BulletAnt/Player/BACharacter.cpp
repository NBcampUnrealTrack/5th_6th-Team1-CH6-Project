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
#include "Components/SpotLightComponent.h"
#include "MotionWarpingComponent.h"
#include "BAAnimInstance.h"
#include "BAParkourComponent.h"
#include "Net/UnrealNetwork.h"
#include "UI/UW_PlayerHUDWidget.h"
#include "UI/UW_Interaction.h"
#include "Weapon/Data/RangedWeaponDataAsset.h"
#include "UI/UISubsystem.h"
#include "UI/UW_Scope.h"
#include "GAS/AttributeSet/HealthAttributeSet.h"
#include "GAS/AttributeSet/AmmoAttributeSet.h"
#include "GAS/AttributeSet/EXPAttributeSet.h"
#include "GAS/BAGameplayTags.h"
#include "Weapon/Sniper/WeaponSniper.h"
//#include "DrawDebugHelpers.h"//디버그 용 빨간 선
#include "Common/BAItemInterface.h"
#include "Weapon/BaseRangedWeapon.h"
#include "Weapon/Data/WeaponDataAsset.h"
#include "Kismet/KismetSystemLibrary.h"
//건축
#include "Building/BuildManagerComponent.h"
#include "Building/BaseShop.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Framework/BAGameState.h"
#include "Player/BAPlayerState.h"
#include "Components/SplineComponent.h"
#include "NiagaraComponent.h"
#include "Components/WidgetComponent.h"
#include "UI/UW_IngameUserInfo.h"

TWeakObjectPtr<USceneCaptureComponent2D> ABACharacter::LocalSceneCapture = nullptr;
const FName ABACharacter::NameReturnEffectColor("User.Color");
const FName ABACharacter::NameReturnPathEffectColor("User.Color");
const FName ABACharacter::NameReturnPathEffectSpawnRate("SpawnRate");

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
	GetCharacterMovement()->PushForceFactor = 0.f;
	GetCharacterMovement()->InitialPushForceFactor = 0.f;

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

	SpotlightComp = CreateDefaultSubobject<USpotLightComponent>(TEXT("SpotlightComp"));
	SpotlightComp->SetupAttachment(RootComponent);

	MotionWarpingComp = CreateDefaultSubobject<UMotionWarpingComponent>(TEXT("MotionWarping"));
	MotionWarpingComp->bAutoActivate = true;
	bIsTurning = false;
	TurnDelayTimer = 0.f;
	SpringArmZ = 0.f;

	//파쿠르
	ParkourComponent = CreateDefaultSubobject<UBAParkourComponent>(TEXT("ParkourComponent"));

	//Test
	//앉기 기능 활성화
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
	GetCharacterMovement()->bUseControllerDesiredRotation = false;
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

	ArrowMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ArrowMesh"));
	ArrowMesh->SetupAttachment(RootComponent);
	ArrowMesh->SetVisibleInSceneCaptureOnly(true);
	ArrowMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ArrowMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	ArrowMesh->SetGenerateOverlapEvents(false);
	ArrowMesh->SetRenderCustomDepth(true);

	USkeletalMeshComponent* CharacterMesh = GetMesh();
	CharacterMesh->SetRenderCustomDepth(true);
	CharacterMesh->CustomDepthStencilValue = 1;

	PathSpline = CreateDefaultSubobject<USplineComponent>(TEXT("PathSpline"));
	PathSpline->SetupAttachment(RootComponent);
	PathSpline->SetAbsolute(true, true, true);

	ReturnEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("ReturnEffect"));
	ReturnEffect->SetupAttachment(RootComponent);
	ReturnEffect->SetVisibility(false);
	ReturnEffect->bAutoActivate = false;

	ReturnPathEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("ReturnPathEffect"));
	ReturnPathEffect->SetupAttachment(PathSpline);

	IngameUserInfoUI = CreateDefaultSubobject<UWidgetComponent>(TEXT("IngameUserInfoUI"));
	IngameUserInfoUI->SetupAttachment(GetMesh(), TEXT("SocketNickname"));
	IngameUserInfoUI->SetWidgetSpace(EWidgetSpace::Screen);
	IngameUserInfoUI->SetUsingAbsoluteRotation(true);
	IngameUserInfoUI->SetRelativeRotation(FRotator::ZeroRotator);
	IngameUserInfoUI->SetDrawAtDesiredSize(true);
	IngameUserInfoUI->SetPivot(FVector2D(0.5f, 1.0f));
	IngameUserInfoUI->SetDrawSize(FVector2D(300.0f, 72.0f));

	GetMesh()->SetCanEverAffectNavigation(false);
}

// Called when the game starts or when spawned
void ABACharacter::BeginPlay()
{
	Super::BeginPlay();

#pragma region Ground

	ResetPath();

	ABAGameState* GS = GetWorld()->GetGameState<ABAGameState>();
	if (IsValid(GS) == true)
	{
		GS->AddActiveCharacter(this);
	}

	OnLevelChanged.AddDynamic(this, &ThisClass::UpdateLevelUI);

	if (IsLocallyControlled() == true)
	{
		InitializeSceneCapture();

		if (IsValid(GS) == true)
		{
			const auto& ActiveCharacters = GS->GetActiveCharacters();
			for (const auto& ActiveCharacter : ActiveCharacters)
			{
				SceneCapture2D->ShowOnlyComponents.Add(ActiveCharacter->ArrowMesh);
			}
		}

		LocalSceneCapture = SceneCapture2D;

		USkeletalMeshComponent* CharacterMesh = GetMesh();
		CharacterMesh->SetCustomDepthStencilValue(0);	
	}
	else
	{
		if (LocalSceneCapture.IsValid() == true)
		{
			LocalSceneCapture->ShowOnlyComponents.Add(ArrowMesh);
		}
	}

#pragma endregion

	LastBodyYaw = GetActorRotation().Yaw;

	if (IsLocallyControlled() == true)
	{
		TArray<USceneComponent*> MeshChildren;
		GetMesh()->GetChildrenComponents(false, MeshChildren);

		USceneComponent* FaceComp = nullptr;
		for (USceneComponent* Child : MeshChildren)
		{
			if (Child->GetName().Contains(TEXT("Face")))
			{
				FaceComp = Child;
				break;
			}
		}

		if (FaceComp)
		{
			TArray<USceneComponent*> FaceAndHair;
			FaceComp->GetChildrenComponents(true, FaceAndHair);
			FaceAndHair.Add(FaceComp);

			for (USceneComponent* Comp : FaceAndHair)
			{
				if (UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Comp))
				{
					HiddenComp.Add(PrimComp);
				}
			}
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("[디버그] HideOnAim 태그가 달린 부위 개수: %d 개입니다!"), HiddenComp.Num());
	PC = Cast<ABAPlayerController>(GetController());

}

void ABACharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	StopInteractionTraceTimer();

	ABAGameState* GS = GetWorld()->GetGameState<ABAGameState>();
	if (IsValid(GS) == true)
	{
		GS->RemoveActiveCharacter(this);
	}

	if (PlayerColorChangeHandle.IsValid() == true)
	{
		ABAPlayerState* PS = GetPlayerState<ABAPlayerState>();
		if (IsValid(PS) == true)
		{
			PS->UnbindOnChangedPlayerColor(PlayerColorChangeHandle);
			PlayerColorChangeHandle.Reset();
		}
	}

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
	
	if (bIsReturning == true)
	{
		if (HasAuthority() == true || IsLocallyControlled() == true)
		{
			HandleReturnMovement(DeltaTime);
		}
	}
	float Speed = GetVelocity().Size2D();

	if (HasAuthority())
	{
		if (Controller)
		{
			ControlRot = Controller->GetControlRotation();
			SyncAimPitch = ControlRot.Pitch;
			SyncAimYaw = ControlRot.Yaw;
		}
	}

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
	if (IsLocallyControlled())
	{
		FCollisionQueryParams Params;
		Params.AddIgnoredActor(this);
		if (EquippedWeapon)
			Params.AddIgnoredActor(EquippedWeapon);
		
		FVector ExactTarget = LineTraceTarget(Params, ECC_GameTraceChannel11);
		Server_UpdateAimTarget(ExactTarget);
	}
	if(ASC && !bIsReturning)
	{
		if (!ASC->HasMatchingGameplayTag(TAG_State_Combat_ADS))
		{
			HidingCharacter(CameraComponent);
			if (bIsAiming)
			{
				SpringArm->TargetArmLength = FMath::FInterpTo(SpringArm->TargetArmLength, 125, DeltaTime, TALengthChangeSpeed);
				CameraComponent->FieldOfView = FMath::FInterpTo(CameraComponent->FieldOfView, AimingFieldOfView, DeltaTime, TALengthChangeSpeed);
			}
			else
			{
				SpringArm->TargetArmLength = FMath::FInterpTo(SpringArm->TargetArmLength, CurrentArmLength, DeltaTime, TALengthChangeSpeed);
				CameraComponent->FieldOfView = FMath::FInterpTo(CameraComponent->FieldOfView, 90.f, DeltaTime, TALengthChangeSpeed);
			}
		}
	}
	if (SpotlightComp && Controller)
	{
		FRotator TargetRot = GetControlRotation();

		FRotator CurrentRot = SpotlightComp->GetComponentRotation();

		FRotator SmoothRot = FMath::RInterpTo(CurrentRot, TargetRot + FRotator(0.f, 5.f, 0.f), DeltaTime, 30.0f);

		SpotlightComp->SetWorldRotation(SmoothRot);
	}
	IdleTurning(DeltaTime);

	UpdateIngameInfoScale();
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

		if (NightVisionAction)
		{
			EnhancedInputComponent->BindAction(NightVisionAction, ETriggerEvent::Started, this, &ABACharacter::ToggleNightVision);
		}

		//상호작용
		//if (InteractionAction)
		//{
		//	EnhancedInputComponent->BindAction(InteractionAction, ETriggerEvent::Started, this, &ABACharacter::Interaction);
		//}

		if (InteractionFAction)
		{
			EnhancedInputComponent->BindAction(InteractionFAction, ETriggerEvent::Started, this, &ABACharacter::Interaction_F);
		}

		if (InteractionQAction)
		{
			EnhancedInputComponent->BindAction(InteractionQAction, ETriggerEvent::Started, this, &ABACharacter::Interaction_Q);
		}

		if (InteractionEAction)
		{
			EnhancedInputComponent->BindAction(InteractionEAction, ETriggerEvent::Started, this, &ABACharacter::Interaction_E);
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
		if (ToggleBuildInfoAction)
		{
			EnhancedInputComponent->BindAction(ToggleBuildInfoAction, ETriggerEvent::Started, this, &ABACharacter::OnToggleBuildInfo);
		}


		// 지하
		if (GroundScannerAction)
		{
			EnhancedInputComponent->BindAction(GroundScannerAction, ETriggerEvent::Started, this, &ABACharacter::SwitchGroundScanner);
		}

		if (PingAction)
		{
			EnhancedInputComponent->BindAction(PingAction, ETriggerEvent::Started, this, &ABACharacter::ExecutePing);
		}

		if (ReturnAction)
		{
			EnhancedInputComponent->BindAction(ReturnAction, ETriggerEvent::Started, this, &ABACharacter::SwitchReturnMode);
		}
	}
}

void ABACharacter::OnRep_Controller()
{
	Super::OnRep_Controller();

	StartInteractionTraceTimer();
}

void ABACharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	if (IsLocallyControlled())
	{
		ABAPlayerState* PS = GetPlayerState<ABAPlayerState>();
		if (PS)
		{
			ASC = PS->GetAbilitySystemComponent();
			ASC->InitAbilityActorInfo(PS, this);

			HealthAttributeSet = PS->GetHealthAttributeSet();
			AmmoAttributeSet = PS->GetAmmoAttributeSet();
			EXPAttributeSet = PS->GetEXPAttributeSet();

			ASC->GetGameplayAttributeValueChangeDelegate(HealthAttributeSet->GetHealthAttribute())
				.AddUObject(this, &ABACharacter::OnHealthChangedCallback);
			ASC->GetGameplayAttributeValueChangeDelegate(AmmoAttributeSet->GetCurrentAmmoAttribute())
				.AddUObject(this, &ABACharacter::OnAmmoChangedCallback);
			ASC->GetGameplayAttributeValueChangeDelegate(AmmoAttributeSet->GetMaxAmmoAttribute())
				.AddUObject(this, &ABACharacter::OnAmmoChangedCallback);
			ASC->GetGameplayAttributeValueChangeDelegate(EXPAttributeSet->GetCurrentEXPAttribute())
				.AddUObject(this, &ABACharacter::OnEXPChangedCallback);
			ASC->GetGameplayAttributeValueChangeDelegate(EXPAttributeSet->GetCurrentLevelAttribute())
				.AddUObject(this, &ABACharacter::OnLevelChangedCallback);

			ASC->RegisterGameplayTagEvent(
				TAG_State_Combat_Dead,
				EGameplayTagEventType::NewOrRemoved
			).AddUObject(this, &ABACharacter::HandleRespawnUI);

		}
	}

	SetPlayerColor();
	UpdateNicknameUI();
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
	if (!ASC) return;
	if (!EquippedWeapon) return;
	if (ASC->HasMatchingGameplayTag(TAG_Weapon_Equipped_Ranged) && ASC->HasMatchingGameplayTag(TAG_State_Combat_Cooldown)) return;
	if (ASC->HasMatchingGameplayTag(TAG_Weapon_Equipped_Jetpack))
		bIsJetPack = true;

	FGameplayTagContainer Tag;

	UWeaponDataAsset* WeaponData = EquippedWeapon->GetWeaponData();
	if (!WeaponData) return;
	Tag.AddTag(WeaponData->WeaponTag);
	ASC->TryActivateAbilitiesByTag(Tag);
}

void ABACharacter::Reload(const FInputActionValue& Value)
{
	if (!ASC) return;

	FGameplayTagContainer Tag;
	Tag.AddTag(TAG_Ability_Active_Reload);

	ASC->TryActivateAbilitiesByTag(Tag);
}

void ABACharacter::StopAttack(const FInputActionValue& Value)
{
	if (!ASC) return;
	if (!EquippedWeapon || !EquippedWeapon->bAutoActive) return;
	if (ASC->HasMatchingGameplayTag(TAG_Weapon_Equipped_Jetpack))
		bIsJetPack = false;
	FGameplayTagContainer CancelTags;
	FGameplayTagContainer IgnoreTags;
	CancelTags.AddTag(TAG_Ability_Active);
	IgnoreTags.AddTag(TAG_Ability_Active_Reload);
	IgnoreTags.AddTag(TAG_Ability_Active_ADS);

	ASC->CancelAbilities(&CancelTags, &IgnoreTags);
}

void ABACharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	SetPlayerColor();
	UpdateNicknameUI();

	ABAPlayerState* PS = GetPlayerState<ABAPlayerState>();
	if (PS)
	{
		ASC = PS->GetAbilitySystemComponent();
		ASC->InitAbilityActorInfo(PS, this);
		PS->InitAbility();

		HealthAttributeSet = PS->GetHealthAttributeSet();
		AmmoAttributeSet = PS->GetAmmoAttributeSet();
		EXPAttributeSet = PS->GetEXPAttributeSet();
		HealthAttributeSet->InitValue(100.f, 150.f);

		
		ASC->GetGameplayAttributeValueChangeDelegate(EXPAttributeSet->GetCurrentLevelAttribute())
			.AddUObject(this, &ABACharacter::OnLevelChangedCallback);

		if (IsLocallyControlled())
		{
			ASC->GetGameplayAttributeValueChangeDelegate(HealthAttributeSet->GetHealthAttribute())
				.AddUObject(this, &ABACharacter::OnHealthChangedCallback);
			ASC->GetGameplayAttributeValueChangeDelegate(AmmoAttributeSet->GetCurrentAmmoAttribute())
				.AddUObject(this, &ABACharacter::OnAmmoChangedCallback);
			ASC->GetGameplayAttributeValueChangeDelegate(AmmoAttributeSet->GetMaxAmmoAttribute())
				.AddUObject(this, &ABACharacter::OnAmmoChangedCallback);
			ASC->GetGameplayAttributeValueChangeDelegate(EXPAttributeSet->GetCurrentEXPAttribute())
				.AddUObject(this, &ABACharacter::OnEXPChangedCallback);
			

			ASC->RegisterGameplayTagEvent(
				TAG_State_Combat_Dead,
				EGameplayTagEventType::NewOrRemoved
			).AddUObject(this, &ABACharacter::HandleRespawnUI);
		}
	}

	if (DefaultWeaponClass)
	{
		UpdateAmmo(OwnedEquipment[0]);
		Server_EquipWeapon(DefaultWeaponClass);
	}

#pragma region InteractionUI
	StartInteractionTraceTimer();
#pragma endregion
}


UAbilitySystemComponent* ABACharacter::GetAbilitySystemComponent() const
{
	ABAPlayerState* PS = GetPlayerState<ABAPlayerState>();
	return PS ? PS->GetAbilitySystemComponent() : nullptr;

}

void ABACharacter::Server_EquipWeapon_Implementation(TSubclassOf<ABaseWeapon> WeaponClass)
{
	if (!HasAuthority()) return;
	if (!WeaponClass) return;
	if (EquippedWeapon && EquippedWeapon->GetClass() == WeaponClass) return;
	

	if (EquippedWeapon)
	{
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
	NewWeapon->SetActorRelativeTransform(NewWeapon->GripOffset);
	NewWeapon->EquipWeapon(ASC);
	EquippedWeapon = NewWeapon;

	if (IsLocallyControlled())
	{
		OnRep_EquippedWeapon();
	}
}

FVector ABACharacter::GetFireStartLocation_Implementation() const
{
	if (EquippedWeapon)
	{
		if (ABaseRangedWeapon* Weapon = Cast<ABaseRangedWeapon>(EquippedWeapon))
		{
			USkeletalMeshComponent* WeaponMesh = Weapon->GetWeaponMesh();
			if (WeaponMesh && WeaponMesh->DoesSocketExist(Weapon->GetMuzzleSocketName()))
			{
				return WeaponMesh->GetSocketLocation(Weapon->GetMuzzleSocketName());
			}
		}
	}

	return GetActorLocation();
}

FVector ABACharacter::GetFireDirection_Implementation() const
{
	if (EquippedWeapon)
	{
		if (ABaseRangedWeapon* Weapon = Cast<ABaseRangedWeapon>(EquippedWeapon))
		{
			USkeletalMeshComponent* WeaponMesh = Weapon->GetWeaponMesh();
			if (WeaponMesh && WeaponMesh->DoesSocketExist(Weapon->GetMuzzleSocketName()))
			{
				if (ASC && ASC->HasMatchingGameplayTag(TAG_State_Combat_ADS))
				{
					return WeaponMesh->GetSocketRotation(Weapon->GetMuzzleSocketName()).Vector().GetSafeNormal();
				}

				FVector WeaponSocketLocation = WeaponMesh->GetSocketLocation(Weapon->GetMuzzleSocketName());
				FVector ViewLoc;
				FRotator ViewRot;
				GetActorEyesViewPoint(ViewLoc, ViewRot);

				FVector AimEnd = ViewLoc + ViewRot.Vector() * 1000000.f;

				return (AimEnd - WeaponSocketLocation).GetSafeNormal();
			}
		}
	}

	return GetActorRotation().Vector().GetSafeNormal();
}

void ABACharacter::StartAiming()
{
	if (!ASC->HasMatchingGameplayTag(TAG_Weapon_Equipped_Ranged)) return;
	bIsAiming = true;

	GetCharacterMovement()->MaxWalkSpeed = UpdateMovementSpeed();
	Server_SetAiming(true);
}

void ABACharacter::EndAiming()
{
	if (!bIsAiming) return;
	bIsAiming = false;

	GetCharacterMovement()->MaxWalkSpeed = UpdateMovementSpeed();

	Server_SetAiming(false);
}

void ABACharacter::OnRep_EquippedWeapon()
{
	if (IsLocallyControlled() && IsValid(EquippedWeapon))
	{
		EquippedWeapon->SetStencilValue(0);
	}
}

void ABACharacter::Server_SetChangeWeapon_Implementation(TSubclassOf<ABaseWeapon> InWeapon, int32 WeaponIndex)
{
	OwnedEquipment[WeaponIndex] = InWeapon;
	Server_EquipWeapon(InWeapon);

	UpdateAmmo(InWeapon);
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

void ABACharacter::OnHealthChangedCallback(const FOnAttributeChangeData& Data) const
{
	OnHealthChanged.Broadcast(HealthAttributeSet->GetHealth(), HealthAttributeSet->GetMaxHealth());
}


void ABACharacter::OnAmmoChangedCallback(const FOnAttributeChangeData& Data) const
{
	OnAmmoChanged.Broadcast(AmmoAttributeSet->GetCurrentAmmo(), AmmoAttributeSet->GetMaxAmmo());
}

void ABACharacter::OnEXPChangedCallback(const FOnAttributeChangeData& Data) const
{
	OnEXPChanged.Broadcast(EXPAttributeSet->GetCurrentEXP(), EXPAttributeSet->GetMaxEXP());
}

void ABACharacter::OnLevelChangedCallback(const FOnAttributeChangeData& Data)
{
	OnLevelChanged.Broadcast(Data.NewValue, Data.OldValue);

	if (HasAuthority())
	{
		if ((int32)Data.NewValue > (int32)Data.OldValue)
		{
			for (int32 i = Data.OldValue; i < Data.NewValue; ++i)
			{
				LevelUp();
			}
		}
	}
}

void ABACharacter::OnDropCallback()
{
	OnDropDelegate.Broadcast(this);
}

void ABACharacter::SetRecoil(float InPitch, float InYaw)
{
	if (!IsValid(this)) return;
	CurrentRecoilPitch = InPitch;
	CurrentRecoilYaw = InYaw;
}

void ABACharacter::HandleRespawnUI(FGameplayTag Tag, int32 NewCount)
{
	if (NewCount >= 1)
	{
		PC->StartRespawnBar(RespawnTime);
	}
}

void ABACharacter::UpdateInteractionTrace()
{
	FVector CamLoc;
	FRotator CamRot;
	GetController()->GetPlayerViewPoint(CamLoc, CamRot);

	FVector Start = CamLoc;
	FVector End = Start + (CamRot.Vector() * LineTraceRange);

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	bool bHit = GetWorld()->LineTraceSingleByChannel(
		Hit,
		Start,
		End,
		ECC_GameTraceChannel4,
		Params
	);

	AActor* NewActor = nullptr;

	if (bHit)
	{
		AActor* HitActor = Hit.GetActor();
		if (HitActor && HitActor->GetClass()->ImplementsInterface(UBAItemInterface::StaticClass()))
		{
			NewActor = HitActor;
		}
	}

	SetCurrentInteractActor(NewActor);
}

void ABACharacter::SetCurrentInteractActor(AActor* NewActor)
{
	if (CurrentInteractActor.Get() == NewActor)
	{
		UpdateInteractionUI();
		return;
	}

	CurrentInteractActor = NewActor;
	UpdateInteractionUI();
}

void ABACharacter::UpdateInteractionUI()
{
	APlayerController* FPC = Cast<APlayerController>(GetWorld()->GetFirstPlayerController());
	if (!GetWorld()) return;
	ULocalPlayer* LP = FPC->GetLocalPlayer();
	UUISubsystem* UISubsystem = LP->GetSubsystem<UUISubsystem>();
	if (!IsValid(UISubsystem))
	{
		return;
	}

	InteractionWidget = UISubsystem->ShowUI<UUW_Interaction>(EUIType::Interaction);
	AActor* Target = CurrentInteractActor.Get();

	if (!Target)
	{
		UISubsystem->HideUI(EUIType::Interaction);
		InteractionWidget->ClearInteraction();
		return;
	}

	TArray<FInteractionOption> Options;
	IBAItemInterface::Execute_GetInteractionOptions(Target, this, Options);

	if (Options.Num() == 0)
	{
		UISubsystem->HideUI(EUIType::Interaction);
		return;
	}

	InteractionWidget->SetInteractionOptions(Options);
}

void ABACharacter::StartInteractionTraceTimer()
{
	if (!IsLocallyControlled())
	{
		return;
	}

	if (!GetController())
	{
		return;
	}

	if (!GetWorldTimerManager().IsTimerActive(InteractionTraceTimerHandle))
	{
		GetWorldTimerManager().SetTimer(
			InteractionTraceTimerHandle,
			this,
			&ABACharacter::UpdateInteractionTrace,
			0.05f,
			true
		);
	}
}

void ABACharacter::StopInteractionTraceTimer()
{
	GetWorldTimerManager().ClearTimer(InteractionTraceTimerHandle);
}

//조준 시작
void ABACharacter::AimStart(const FInputActionValue& Value)
{
	StartAiming();
}

//조준 끝
void ABACharacter::AimStop(const FInputActionValue& Value)
{
	EndAiming();
}

void ABACharacter::ADSStart(const FInputActionValue& Value)
{
	if (!ASC) return;
	if (!EquippedWeapon) return;

	FGameplayEventData Payload;
	Payload.OptionalObject = EquippedWeapon;
	if (!ASC->HasMatchingGameplayTag(TAG_State_Combat_ADS))
	{
		ASC->HandleGameplayEvent(TAG_Ability_Active_ADS, &Payload);
	}
	else
	{
		ASC->HandleGameplayEvent(TAG_Event_Combat_EndADS, &Payload);
	}
}

void ABACharacter::ToggleNightVision(const FInputActionValue& Value)
{
	if (!EquippedWeapon->GetWeaponData() || !(EquippedWeapon->GetWeaponData()->WeaponType == EWeaponType::Sniper)) return;
	if (!ASC || !ASC->HasMatchingGameplayTag(TAG_State_Combat_ADS)) return;

	AWeaponSniper* Sniper = Cast<AWeaponSniper>(EquippedWeapon);
	if (bIsNightVision)
	{
		Sniper->StopNightVision();
	}
	else
	{
		Sniper->StartNightVision();
	}

	bIsNightVision = !bIsNightVision;
}


void ABACharacter::Server_SetAiming_Implementation(bool bNewIsAiming)
{
	bIsAiming = bNewIsAiming;

	GetCharacterMovement()->MaxWalkSpeed = UpdateMovementSpeed();
}

void ABACharacter::Interaction(const FInputActionValue& Value)
{
	if (ASC->HasMatchingGameplayTag(TAG_State_Combat_Dead)) return;

	FVector CamLoc;
	FRotator CamRot;
	GetController()->GetPlayerViewPoint(CamLoc, CamRot);;
	FVector Start = CamLoc;
	FVector End = Start + (CamRot.Vector() * LineTraceRange);

	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	bool bHit = GetWorld()->LineTraceSingleByChannel(
		HitResult,
		Start,
		End,
		ECC_GameTraceChannel4,
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

void ABACharacter::Interaction_F(const FInputActionValue& Value)
{
	TryInteractionByKey(EKeys::F);
}

void ABACharacter::Interaction_Q(const FInputActionValue& Value)
{
	TryInteractionByKey(EKeys::Q);
}

void ABACharacter::Interaction_E(const FInputActionValue& Value)
{
	TryInteractionByKey(EKeys::E);
}

void ABACharacter::TryInteractionByKey(const FKey& PressedKey)
{
	AActor* Target = CurrentInteractActor.Get();
	if (!Target)
	{
		return;
	}

	if (!Target->GetClass()->ImplementsInterface(UBAItemInterface::StaticClass()))
	{
		return;
	}

	TArray<FInteractionOption> Options;
	IBAItemInterface::Execute_GetInteractionOptions(Target, this, Options);

	for (const FInteractionOption& Option : Options)
	{
		if (Option.Key == PressedKey)
		{
			IBAItemInterface::Execute_Interaction(Target, this, Option.ActionName);
			return;
		}
	}

	//if (PressedKey == EKeys::F)
	//{
	//	IBAItemInterface::Execute_Use(Target, this);
	//}
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

void ABACharacter::OnToggleBuildInfo(const FInputActionValue& Value)
{
	BuildManager->ToggleBuildMenu();
}

void ABACharacter::StartSwitchWeapon(const FInputActionValue& Value)
{
	int32 Index = (int32)Value.Get<float>() - 1;
	if (!OwnedEquipment.IsValidIndex(Index)) return;

	if (Index == 0)
	{
		if (IsValid(PC))
		{
			if (UUW_PlayerHUDWidget* HUD = PC->GetHUD())
			{
				ABaseWeapon* WeaponCDO = OwnedEquipment[0]->GetDefaultObject<ABaseWeapon>();
				URangedWeaponDataAsset* Data = Cast<URangedWeaponDataAsset>(WeaponCDO->GetWeaponData());
				if (Data)
				{
					if (Data->bAutoFire)
						HUD->SetAutoImage(true);
					else
						HUD->SetAutoImage(false);

					PC->ShowAmmo();
				}
			}
		}
	}
	else
	{
		if (IsValid(PC))
		{
			PC->HideAmmo();
		}
	}

	Server_EquipWeapon(OwnedEquipment[Index]);
}

void ABACharacter::JumpHandler(const FInputActionValue& Value)
{
	bool bParkourStarted = false;

	StopMontage();
	if (ParkourComponent)
	{
		bParkourStarted = ParkourComponent->AttemptParkour();
	}

	if (!bParkourStarted)
	{
		Jump();
	}
}
void ABACharacter::ExecutePing(const FInputActionValue& Value)
{
	if (IsValid(ASC) == false)
		return;

	FGameplayTagContainer Tag;
	Tag.AddTag(TAG_Event_Communicate_Ping);

	ASC->TryActivateAbilitiesByTag(Tag);
}

void ABACharacter::SwitchReturnMode(const FInputActionValue& Value)
{
	if (bIsReturning == false)
	{
		StartReturning();
	}
	else
	{
		StopReturning();
	}
}

void ABACharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABACharacter, bIsAiming);
	DOREPLIFETIME(ABACharacter, bIsRunning);
	DOREPLIFETIME(ABACharacter, bIsTurning);
	DOREPLIFETIME(ABACharacter, SyncAimYaw);
	DOREPLIFETIME(ABACharacter, SyncAimPitch);
	DOREPLIFETIME(ABACharacter, EquippedWeapon);
	DOREPLIFETIME(ABACharacter, OwnedEquipment);
	DOREPLIFETIME_CONDITION_NOTIFY(ABACharacter, bIsReturning, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME(ABACharacter, ReplicatedAimTarget);
	DOREPLIFETIME(ABACharacter, bIsJetPack);
}

void ABACharacter::Multicast_PlayTurnMontage_Implementation(UAnimMontage* MontageToPlay, FTransform TargetTransform)
{
	if (!MontageToPlay) return;
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (!AnimInstance) return;

	if (AnimInstance->Montage_IsPlaying(MontageToPlay)) return;
	if (MotionWarpingComp)
	{
		MotionWarpingComp->AddOrUpdateWarpTargetFromTransform(FName("TurnTarget"), TargetTransform);
	}

	if (AnimInstance && MontageToPlay)
	{
		float PlayResult = AnimInstance->Montage_Play(MontageToPlay);

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
	//if (MotionWarpingComp)
	//{
		//MotionWarpingComp->DisableAllRootMotionModifiers();
	//}
	LastBodyYaw = GetActorRotation().Yaw;
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
	if (ParkourComponent->bIsParkour || GetVelocity().Size2D() > 1.f || GetCharacterMovement()->IsFalling())
	{
		LastBodyYaw = GetActorRotation().Yaw;
		RootYawOffset = 0.f;
		return;
	}
	//if (GEngine && (HasAuthority() || IsLocallyControlled()))
	//{
		//GEngine->AddOnScreenDebugMessage(1, 0.0f, FColor::Red, FString::Printf(TEXT("현재 각도: %f"), RootYawOffset));
	//}
	RootYawOffset = UKismetMathLibrary::NormalizeAxis(LastBodyYaw - GetActorRotation().Yaw);
	if (!HasAuthority() && !IsLocallyControlled()) return;
	if (!bIsTurning)
	{
		if (FMath::Abs(RootYawOffset) > 55.f)
		{
			TurnDelayTimer += DeltaTime;
			if (TurnDelayTimer > 0.1f)
			{
				bIsTurning = true;
				TurnStartYaw = LastBodyYaw;

				bool bRight = RootYawOffset < 0;
				TurnType = bRight ? ETurnType::Right90 : ETurnType::Left90;
				CurrentTurnSpeed = 10.f;

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
	}
	else
	{
		float TotalFlickYaw = UKismetMathLibrary::NormalizeAxis(GetBaseAimRotation().Yaw - TurnStartYaw);

		if (FMath::Abs(TotalFlickYaw) > 160.f && (TurnType == ETurnType::Right90 || TurnType == ETurnType::Left90))
		{
			UE_LOG(LogTemp, Warning, TEXT("180도 감지. 90도 취소하고 180도로 덮어씌움."));
			bool bRight = (TotalFlickYaw < 0); // 방향 다시 계산
			TurnType = bRight ? ETurnType::Right180 : ETurnType::Left180;
			if (bIsCrouched)
			{
				switch (TurnType)
				{
				case ETurnType::Left180:
					CurrentTurnMontage = CrouchTurnLeft180Montage;
					break;
				case ETurnType::Right180:
					CurrentTurnMontage = CrouchTurnRight180Montage;
					break;
				}
			}
			else
			{
				switch (TurnType)
				{
				case ETurnType::Left180:
					CurrentTurnMontage = TurnLeft180Montage;
					break;
				case ETurnType::Right180:
					CurrentTurnMontage = TurnRight180Montage;
					break;
				}
			}
			CurrentTurnSpeed = 15.f;

			if (CurrentTurnMontage && MotionWarpingComp)
			{
				FRotator GoalRot = FRotator(0.f, GetControlRotation().Yaw, 0.f);
				FTransform TargetTransform(GoalRot, GetActorLocation());
				Multicast_PlayTurnMontage(CurrentTurnMontage, TargetTransform);
			}
		}
	}
}

void ABACharacter::SetTurnStatus()
{
	bIsTurning = false;
	TurnType = ETurnType::None;
	CurrentTurnMontage = nullptr;
	TurnDelayTimer = 0.f;
}

void ABACharacter::StopMontage()
{
	if (bIsTurning)
	{
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

void ABACharacter::Server_UpdateAimTarget_Implementation(FVector_NetQuantize NewTarget)
{
	ReplicatedAimTarget = NewTarget;
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

	if (PC)
	{
		PC->SwitchGroundScanner();
	}
}

void ABACharacter::RequestWeaponLog(UWeaponDataAsset* InData)
{
	if (!HasAuthority()) return;
	Multicast_ShowWeaponLog(InData);
}

void ABACharacter::Multicast_ShowWeaponLog_Implementation(UWeaponDataAsset* InData)
{
	APlayerController* FPC = Cast<APlayerController>(GetWorld()->GetFirstPlayerController());
	if (!GetWorld()) return;
	ULocalPlayer* LP = FPC->GetLocalPlayer();
	if (!LP) return;

	UUISubsystem* UISubsystem = LP->GetSubsystem<UUISubsystem>();
	if (IsValid(UISubsystem))
	{
		UUW_PlayerHUDWidget* HUD = UISubsystem->ShowUI<UUW_PlayerHUDWidget>(EUIType::PlayerHUD);
		if (HUD)
		{
			HUD->AddWeaponLog(InData);
		}
	}
}

void ABACharacter::InitializeSceneCapture()
{
	SceneCapture2D = NewObject<USceneCaptureComponent2D>(this, TEXT("SceneCapture2D"));
	SceneCapture2D->RegisterComponent();
	SceneCapture2D->AttachToComponent(SceneCaptureParent, FAttachmentTransformRules::KeepRelativeTransform);

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

	SceneCapture2D->PostProcessSettings.AddBlendable(M_PostProcessGroundScanner, 1.0f);
	SceneCapture2D->TextureTarget = RT_GroundScanner;
}

void ABACharacter::UpdateShowComponents()
{
	SceneCapture2D->ShowOnlyComponents.Add(ArrowMesh);
}

void ABACharacter::SetPlayerColor()
{
	ABAPlayerState* PS = GetPlayerState<ABAPlayerState>();
	if (IsValid(PS) == true)
	{
		FLinearColor PlayerColor = PS->GetPlayerColor();
		if (IsValid(ArrowMesh) == true)
		{
			ArrowMesh->SetCustomPrimitiveDataVector4(0, FVector4(PlayerColor));
		}

		if (IsValid(ReturnEffect) == true)
		{
			ReturnEffect->SetVariableLinearColor(NameReturnEffectColor, PlayerColor * 3.0f);
		}

		if (IsValid(ReturnPathEffect) == true)
		{
			ReturnPathEffect->SetVariableLinearColor(NameReturnPathEffectColor, PlayerColor * 3.0f);
		}

		if (IsValid(IngameUserInfoUI) == true)
		{
			// 호스트는 위젯 생성 시점의 문제로 가끔 위젯이 비어있는 경우가 있음. 강제로 생성.
			IngameUserInfoUI->InitWidget();

			UUW_IngameUserInfo* UserInfoUI = Cast<UUW_IngameUserInfo>(IngameUserInfoUI->GetWidget());
			if (IsValid(UserInfoUI) == true)
			{
				UserInfoUI->SetColor(PlayerColor);
			}
		}

		if (PlayerColorChangeHandle.IsValid() == true)
			return;

		PlayerColorChangeHandle = PS->BindOnChangedPlayerColor(FOnChangedPlayerColor::FDelegate::CreateLambda(
			[WeakThis = TWeakObjectPtr(this)](FLinearColor NewColor)
			{
				if (WeakThis.IsValid() == true)
				{
					WeakThis->SetPlayerColor();
				}
			}));
	}
}

void ABACharacter::Server_SetIsInZone_Implementation(bool bInZone)
{
	if (bIsInReturnZone == bInZone)
		return;

	bIsInReturnZone = bInZone;
	
	if (bIsInReturnZone == true)
	{
		Multicast_ResetPath();

		FVector EnterLoc = GetActorLocation();
		FVector AirPoint = EnterLoc + FVector(0.0f, 0.0f, ExitAirHeight);
		FVector RandomDir = FRotator(0.0f, FMath::FRandRange(0.0f, 360.0f), 0.0f).Vector();
		FVector LandingPoint = AirPoint + RandomDir * 600.0f - FVector(0.0f, 0.0f, 200.0f);

		Multicast_AddPathPoint(LandingPoint);
		Multicast_AddPathPoint(AirPoint);
		Multicast_AddPathPoint(EnterLoc);
	}
	else
	{
		Server_StopRecordingPath();
	}
}

void ABACharacter::ResetPath()
{
	PathSpline->ClearSplinePoints();

	PathSpline->UpdateSpline();
	int32 TotalPoints = PathSpline->GetNumberOfSplinePoints();
	ReturnPathEffect->SetVariableFloat(NameReturnPathEffectSpawnRate, TotalPoints);
}

void ABACharacter::StartRecordingPath()
{
	Server_StartRecordingPath();
}

void ABACharacter::StopRecordingPath()
{
	Server_StopRecordingPath();
}

void ABACharacter::UpdateSplinePath()
{
	if (bIsReturning == true || HasAuthority() == false || bIsInReturnZone == false)
		return;

	FVector CurrLocation = GetActorLocation();
	int32 TotalPoints = PathSpline->GetNumberOfSplinePoints();
	if (TotalPoints == 0)
	{
		Multicast_AddPathPoint(CurrLocation);
		return;
	}
	;
	FVector LastPointLocation = PathSpline->GetLocationAtSplinePoint(TotalPoints - 1, ESplineCoordinateSpace::World);

	float ThresholdSquared = PathDistThreshold * PathDistThreshold;
	if (FVector::DistSquared(CurrLocation, LastPointLocation) > ThresholdSquared)
	{
		Multicast_AddPathPoint(CurrLocation);
	}
}

void ABACharacter::Server_ResetPath_Implementation()
{
	Multicast_ResetPath();
}

void ABACharacter::Multicast_ResetPath_Implementation()
{
	ResetPath();
}

void ABACharacter::Server_StartRecordingPath_Implementation()
{
	if (bIsInReturnZone == false)
		return;

	Multicast_AddPathPoint(GetActorLocation());

	GetWorldTimerManager().SetTimer(
		PathUpdateTimer,
		this,
		&ThisClass::UpdateSplinePath,
		0.2f,
		true);
}

void ABACharacter::Server_StopRecordingPath_Implementation()
{
	GetWorldTimerManager().ClearTimer(PathUpdateTimer);
}

void ABACharacter::AddPathPoint(FVector NewPoint)
{
	PathSpline->AddSplinePoint(NewPoint, ESplineCoordinateSpace::World, false);

	PathSpline->UpdateSpline();
	int32 TotalPoints = PathSpline->GetNumberOfSplinePoints();
	ReturnPathEffect->SetVariableFloat(NameReturnPathEffectSpawnRate, TotalPoints);
}

void ABACharacter::RemovePathPoints(int32 LastIdx)
{
	if (LastIdx < 0)
	{
		ResetPath();
	}
	else
	{
		int32 TotalPoints = PathSpline->GetNumberOfSplinePoints();
		for (int32 Idx = TotalPoints - 1; Idx > LastIdx; --Idx)
		{
			PathSpline->RemoveSplinePoint(Idx, false);
		}

		PathSpline->UpdateSpline();
		TotalPoints = PathSpline->GetNumberOfSplinePoints();
		ReturnPathEffect->SetVariableFloat(NameReturnPathEffectSpawnRate, TotalPoints);
	}
}

int32 ABACharacter::GetRemainedPathIdx(float FinalDistance)
{
	if (FinalDistance == 0.0f)
		return -1;

	int32 TotalPoints = PathSpline->GetNumberOfSplinePoints();
	for (int32 Idx = TotalPoints - 1; Idx >= 0; --Idx)
	{
		float PointDistance = PathSpline->GetDistanceAlongSplineAtSplinePoint(Idx);

		if (PointDistance <= FinalDistance)
			return Idx + 1;
	}

	return TotalPoints;
}

void ABACharacter::Multicast_AddPathPoint_Implementation(FVector NewPoint)
{
	AddPathPoint(NewPoint);
}

void ABACharacter::Multicast_RemovePoints_Implementation(int32 LastIdx)
{
	RemovePathPoints(LastIdx);
}

void ABACharacter::SetIsReturning(bool bInReturning)
{
	if (IsValid(ASC) == true)
	{
		if (bInReturning == true)
		{
			FGameplayTagContainer Container;
			Container.AddTag(TAG_State);
			Container.AddTag(TAG_Event_Weapon_Switch);
			if (ASC->HasAnyMatchingGameplayTags(Container) == true)
				return;

			Container.RemoveTag(TAG_State_Combat_Dead);
			ASC->BlockAbilitiesWithTags(Container);
		}
		else
		{
			FGameplayTagContainer Container;
			Container.AddTag(TAG_State);
			Container.AddTag(TAG_Event_Weapon_Switch);
			ASC->UnBlockAbilitiesWithTags(Container);
		}
	}

	bIsReturning = bInReturning;
	OnRep_IsReturning();
}

void ABACharacter::StartReturning()
{
	if (bIsReturning == true || PathSpline->GetNumberOfSplinePoints() < 2)
		return;

	ReturnDistance = PathSpline->GetSplineLength();

	SetIsReturning(true);		// 미리 ReturnPoint 더 기록되는 거 막음
	Server_StartReturning();
}

void ABACharacter::HandleReturnMovement(float DeltaTime)
{
	ReturnDistance -= ReturnSpeed * DeltaTime;
	if (ReturnDistance <= 0.0f)
	{
		ReturnDistance = 0.0f;
		if (HasAuthority() == true)
		{
			Server_StopReturning();
		}
	}

	FVector TargetLoc = PathSpline->GetLocationAtDistanceAlongSpline(ReturnDistance, ESplineCoordinateSpace::World);
	FRotator TargetRot = PathSpline->GetRotationAtDistanceAlongSpline(ReturnDistance, ESplineCoordinateSpace::World);
	TargetRot.Pitch = 0.0f;
	TargetRot.Roll = 0.0f;

	SetActorLocationAndRotation(TargetLoc, TargetRot, false, nullptr, ETeleportType::TeleportPhysics);

	if (HasAuthority() == true)
	{
		int32 LastIdx = GetRemainedPathIdx(ReturnDistance);
		Multicast_RemovePoints(LastIdx);
	}
}

void ABACharacter::StopReturning()
{
	if (bIsReturning == false || bIsInReturnZone == false)
		return;

	SetIsReturning(false);
	Server_StopReturning();
}

void ABACharacter::Server_StartReturning_Implementation()
{
	ReturnDistance = PathSpline->GetSplineLength();

	SetIsReturning(true);
}

void ABACharacter::Server_StopReturning_Implementation()
{
	SetIsReturning(false);
}

void ABACharacter::ActivateReturnEffect()
{
	USkeletalMeshComponent* CharacterMesh = GetMesh();
	if (IsValid(CharacterMesh) == true)
	{
		CharacterMesh->SetVisibility(false);
		CharacterMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		CharacterMesh->SetActive(false);
		CharacterMesh->SetComponentTickEnabled(false);
	}
	if (IsValid(EquippedWeapon) == true)
	{
		EquippedWeapon->SetActorHiddenInGame(true);
	}
	SpringArm->SetRelativeLocation(ReturnArmLoaction);
	SpringArm->TargetArmLength = ReturnArmLength;
	SpringArm->SocketOffset = ReturnSocketOffset;
	ReturnEffect->SetVisibility(true);
	ReturnEffect->Activate();
}

void ABACharacter::DeactivateReturnEffect()
{
	USkeletalMeshComponent* CharacterMesh = GetMesh();
	if (IsValid(CharacterMesh) == true)
	{
		CharacterMesh->SetVisibility(true);
		CharacterMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		CharacterMesh->SetActive(true);
		CharacterMesh->SetComponentTickEnabled(true);
	}
	if (IsValid(EquippedWeapon) == true)
	{
		EquippedWeapon->SetActorHiddenInGame(false);
	}
	SpringArm->SetRelativeLocation(DefaultSpringArmLocation);
	SpringArm->TargetArmLength = DefaultSpringArmLength;
	SpringArm->SocketOffset = DefaultSpringArmSocektOffset;
	ReturnEffect->SetVisibility(false);
	ReturnEffect->Deactivate();
}

void ABACharacter::OnRep_IsReturning()
{
	if (bIsReturning == true)
	{
		ActivateReturnEffect();
		GetCharacterMovement()->NetworkSimulatedSmoothLocationTime = 0.15f;
		GetCharacterMovement()->bIgnoreClientMovementErrorChecksAndCorrection = true;
		GetCharacterMovement()->SetMovementMode(MOVE_Flying);
		//GetCharacterMovement()->bCheatFlying = true;
		SetActorEnableCollision(false);
	}
	else
	{
		DeactivateReturnEffect();
		GetCharacterMovement()->NetworkSimulatedSmoothLocationTime = 0.05f;		// 기본값
		GetCharacterMovement()->bIgnoreClientMovementErrorChecksAndCorrection = false;
		GetCharacterMovement()->SetMovementMode(MOVE_Walking);
		//GetCharacterMovement()->bCheatFlying = false;
		SetActorEnableCollision(true);
	}
}

void ABACharacter::UpdateNicknameUI()
{
	if (IsValid(IngameUserInfoUI) == false)
		return;

	ABAPlayerState* PS = GetPlayerState<ABAPlayerState>();
	if (IsValid(PS) == false)
		return;

	UUW_IngameUserInfo* UserInfoUI = Cast<UUW_IngameUserInfo>(IngameUserInfoUI->GetWidget());
	if (IsValid(UserInfoUI) == false)
		return;

	UserInfoUI->SetNickname(PS->GetPlayerName());
}

void ABACharacter::UpdateLevelUI(float CurrentLevel, float OldLevel)
{
	if (IsValid(IngameUserInfoUI) == false)
		return;

	UUW_IngameUserInfo* UserInfoUI = Cast<UUW_IngameUserInfo>(IngameUserInfoUI->GetWidget());
	if (IsValid(UserInfoUI) == false)
		return;

	UserInfoUI->SetLevel((int32)CurrentLevel);
}

void ABACharacter::UpdateIngameInfoScale()
{
	if (IsValid(IngameUserInfoUI) == false)
		return;

	UUW_IngameUserInfo* UserInfoUI = Cast<UUW_IngameUserInfo>(IngameUserInfoUI->GetWidget());
	if (IsValid(UserInfoUI) == false)
		return;

	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
	if (IsValid(PlayerController) == false)
		return;

	APlayerCameraManager* CM = PlayerController->PlayerCameraManager;
	if (IsValid(CM) == false)
		return;

	FVector CameraLoc = CM->GetCameraLocation();
	float Dist = FVector::Dist(CameraLoc, GetActorLocation());

	const float MinDist = 200.0f;
	const float MaxDist = 6000.0f;
	float NewScale = FMath::GetMappedRangeValueClamped(
		FVector2D(MinDist, MaxDist),
		FVector2D(1.0f, 0.0f),
		Dist);

	UserInfoUI->SetScale(NewScale);

	bool bVisible = NewScale > 0.01f;
	IngameUserInfoUI->SetVisibility(bVisible);
}

void ABACharacter::GetEXP(float InEXP)
{
	if (!HasAuthority()) return;
	if (!ASC || !EXPEffectClass) return;
	

	FGameplayEffectContextHandle Context = ASC->MakeEffectContext();

	FGameplayEffectSpecHandle Spec =
		ASC->MakeOutgoingSpec(EXPEffectClass, 1.f, Context);

	if (Spec.IsValid())
	{
		Spec.Data->SetSetByCallerMagnitude(
			TAG_Data_Reward_EXP,
			InEXP
		);

		ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
	}
}

void ABACharacter::LevelUp()
{
	if (!HasAuthority()) return;
	if (!ASC || !LevelUpEffectClass) return;

	FGameplayEffectContextHandle Context = ASC->MakeEffectContext();

	FGameplayEffectSpecHandle Spec =
		ASC->MakeOutgoingSpec(LevelUpEffectClass, 1.f, Context);

	float BaseHealth = HealthAttributeSet->GetHealth();
	float HealthIncrease = BaseHealth * 0.15;

	float BaseAttackPower = HealthAttributeSet->GetAttackPower();
	float AttackPowerIncrease = BaseAttackPower * 0.1f;

	if (Spec.IsValid())
	{
		Spec.Data->SetSetByCallerMagnitude(
			TAG_Data_Reward_IncreaseMaxHealth,
			HealthIncrease
		);

		Spec.Data->SetSetByCallerMagnitude(
			TAG_Data_Reward_IncreaseAttackPower,
			AttackPowerIncrease
		);

		ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
	}
}

void ABACharacter::UpdateAmmo(TSubclassOf<ABaseWeapon> InWeaponClass)
{
	if (ASC)
	{
		const UAmmoAttributeSet* AmmoSet = ASC->GetSet<UAmmoAttributeSet>();
		if (!AmmoSet) return;

		ABaseRangedWeapon* CDOWeapon = Cast<ABaseRangedWeapon>(InWeaponClass->GetDefaultObject());
		if (!CDOWeapon) return;

		URangedWeaponDataAsset* Data = Cast<URangedWeaponDataAsset>(CDOWeapon->GetWeaponData());
		if (!Data) return;

		int32 NewMaxAmmo = Data->MaxAmmo;

		ASC->SetNumericAttributeBase(
			UAmmoAttributeSet::GetMaxAmmoAttribute(),
			NewMaxAmmo
		);

		ASC->SetNumericAttributeBase(
			UAmmoAttributeSet::GetCurrentAmmoAttribute(),
			NewMaxAmmo
		);
	}
}

FVector ABACharacter::LineTraceTarget(FCollisionQueryParams Params, ECollisionChannel ECC)
{
	if (!GetController()) return GetActorLocation() + (GetActorForwardVector() * 1000.f);

	FVector CamLoc;
	FRotator CamRot;
	GetController()->GetPlayerViewPoint(CamLoc, CamRot);

	FVector TraceEnd = CamLoc + (CamRot.Vector() * 10000.0f);

	FHitResult Hit;
	Params.AddIgnoredActor(this);

	bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, CamLoc, TraceEnd, ECC, Params);

	return bHit ? Hit.ImpactPoint : TraceEnd;
}

void ABACharacter::HidingCharacter(UCameraComponent* CameraComp)
{
	if (CameraComp && GetMesh())
	{
		float DistanceToCamera = FVector::Dist(GetActorLocation(), CameraComp->GetComponentLocation());

		float HideThreshold = 150.f;

		if (bIsAiming)
			HideThreshold = 100.f;

		if (DistanceToCamera < HideThreshold)
		{
			GetMesh()->SetOwnerNoSee(true);

			if (EquippedWeapon && EquippedWeapon->GetWeaponMesh())
			{
				EquippedWeapon->GetWeaponMesh()->SetOwnerNoSee(true);
			}
		}
		else
		{
			GetMesh()->SetOwnerNoSee(false);

			if (EquippedWeapon && EquippedWeapon->GetWeaponMesh())
			{
				EquippedWeapon->GetWeaponMesh()->SetOwnerNoSee(false);
			}
		}
	}
}
