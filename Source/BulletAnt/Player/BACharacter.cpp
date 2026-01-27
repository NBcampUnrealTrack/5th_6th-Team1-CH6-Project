// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/BACharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

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

// 입력 바인딩
void ABACharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {

        // 점프
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

        // 조준
        if (AimAction)
        {
            EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Started, this, &ABACharacter::AimStart);
            EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Completed, this, &ABACharacter::AimStop);
        }

        //상호작용
        if (InteractionAction)
        {
            EnhancedInputComponent->BindAction(InteractionAction, ETriggerEvent::Triggered, this, &ABACharacter::Interaction);
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

//조준 시작
void ABACharacter::AimStart(const FInputActionValue& Value)
{
    bIsAiming = true;

    //캐릭터 회전 고정
    bUseControllerRotationYaw = true;

    GetCharacterMovement() -> bOrientRotationToMovement = false;
    GetCharacterMovement()->MaxWalkSpeed = 300.f;
}

//조준 끝
void ABACharacter::AimStop(const FInputActionValue& Value)
{
    bIsAiming = false;

    //캐릭터 회전 고정
    bUseControllerRotationYaw = false;

    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->MaxWalkSpeed = 600.f;
}

void ABACharacter::Interaction(const FInputActionValue& Value)
{

}