#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "AbilitySystemInterface.h"
#include "Common/DataAssetInterface.h"
#include "GameplayEffectTypes.h"
#include "BACharacter.generated.h"

class UHealthAttributeSet;
class ABaseWeapon;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAttributeChangedDelegate, float, CurrentValue, float, MaxValue);

UCLASS()
class BULLETANT_API ABACharacter : public ACharacter, public IAbilitySystemInterface, public IDataAssetInterface
{
	GENERATED_BODY()

public:
	ABACharacter();

protected:
    virtual void BeginPlay() override;

public:
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
    class USpringArmComponent* CameraBoom;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
    class UCameraComponent* FollowCamera;

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

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
    class UInputAction* AttackAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    class UInputAction* AimAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    class UInputAction* InteractionAction;

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

    UPROPERTY()
    FGameplayTag CurrentWeaponAbilityTag;

    virtual void PossessedBy(AController* NewController) override;

    void OnHealthChangedCallback(const FOnAttributeChangeData& Data) const;

#pragma endregion

#pragma region Weapon
public:
    UFUNCTION()
    void EquipWeapon(TSubclassOf<ABaseWeapon> WeaponClass);

    UPROPERTY(EditDefaultsOnly, Category = "Combat|Weapon")
    TSubclassOf<ABaseWeapon> DefaultWeaponClass;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat|Weapon")
    TObjectPtr<ABaseWeapon> EquippedWeapon;

    virtual UDataAsset* GetDataAsset() const override;

#pragma endregion

    void StartRunning(const FInputActionValue& Value);
    void StopRunning(const FInputActionValue& Value);
    void CrouchInput(const FInputActionValue& Value);
    void AimStart(const FInputActionValue& Value);
    void AimStop(const FInputActionValue& Value);
    void Interaction(const FInputActionValue& Value);
    void UpdateMovementSpeed();

public:
    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bIsAiming;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bIsRunning;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float WalkSpeed = 400.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float AimSpeed = 350.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float RunningSpeed = 800.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float CrouchSpeed = 300.f;
};
