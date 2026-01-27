// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "AbilitySystemInterface.h"
#include "GameplayEffectTypes.h"
#include "BACharacter.generated.h"

class UHealthAttributeSet;
class ABaseWeapon;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAttributeChangedDelegate, float, CurrentValue, float, MaxValue);

UCLASS()
class BULLETANT_API ABACharacter : public ACharacter, public IAbilitySystemInterface
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

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
    class UInputAction* AttackAction;

    // --- 실제 동작 함수 ---
protected:
    void Move(const FInputActionValue& Value);
    void Look(const FInputActionValue& Value);
    void Attack(const FInputActionValue& Value);

#pragma region GAS 

public:
    virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

    UPROPERTY(BlueprintAssignable, Category = "Combat|UI")
    FOnAttributeChangedDelegate OnHealthChanged;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
    UAbilitySystemComponent* AbilitySystemComponent;

    UPROPERTY()
    UHealthAttributeSet* HealthAttributeSet;

    UPROPERTY(EditAnywhere, Category = "GAS")
    TArray<TSubclassOf<UGameplayAbility>> DefaultAbility;

    virtual void PossessedBy(AController* NewController) override;

    void OnHealthChangedCallback(const FOnAttributeChangeData& Data) const;

#pragma endregion

#pragma region Weapon
public:
    UFUNCTION()
    void EquipWeapon(TSubclassOf<ABaseWeapon> WeaponClass);

    UPROPERTY(EditDefaultsOnly, Category = "Combat|Weapon")
    TSubclassOf<ABaseWeapon> DefaultWeaponClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Weapon")
    TObjectPtr<ABaseWeapon> EquippedWeapon;

#pragma endregion
};
