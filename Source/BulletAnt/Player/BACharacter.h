// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "BACharacter.generated.h"

UCLASS()
class BULLETANT_API ABACharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ABACharacter();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    // --- 카메라 관련 컴포넌트 ---
protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
    class USpringArmComponent* CameraBoom;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
    class UCameraComponent* FollowCamera;

    // --- 입력(Enhanced Input) 관련 변수 ---
public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
    class UInputAction* JumpAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
    class UInputAction* MoveAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
    class UInputAction* RunAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
    class UInputAction* CrouchAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
    class UInputAction* LookAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    class UInputAction* AimAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    class UInputAction* InteractionAction;

    // --- 실제 동작 함수 ---
protected:
    void Move(const FInputActionValue& Value);
    void Look(const FInputActionValue& Value);
    void StartRunning(const FInputActionValue& Value);
    void StopRunning(const FInputActionValue& Value);
    void CrouchInput(const FInputActionValue& Value);
    void AimStart(const FInputActionValue& Value);
    void AimStop(const FInputActionValue& Value);
    void Interaction(const FInputActionValue& Value);

public:
    //조준상태
    UPROPERTY(BlueprintReadOnly, Category = "Input")
    bool bIsAiming;

    //달리기 상태
    UPROPERTY(BlueprintReadOnly, Category = "Input")
    bool bIsRunning;

    // --- 에디터 수정 가능 ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float WalkSpeed = 400.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float AimSpeed = 350.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float RunningSpeed = 800.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float CrouchSpeed = 300.f;
};
