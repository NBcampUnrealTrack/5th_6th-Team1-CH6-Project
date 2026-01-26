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
    // 에디터에서 할당할 입력 액션 (IA)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
    class UInputAction* JumpAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
    class UInputAction* MoveAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
    class UInputAction* LookAction;

    // --- 실제 동작 함수 ---
protected:
    void Move(const FInputActionValue& Value);
    void Look(const FInputActionValue& Value);
};
