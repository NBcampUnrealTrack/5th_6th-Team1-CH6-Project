#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "AbilitySystemInterface.h"
#include "Common/DataAssetInterface.h"
#include "Common/FireStartInterface.h"
#include "GameplayEffectTypes.h"
#include "BACharacter.generated.h"

class UHealthAttributeSet;
class ABaseWeapon;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAttributeChangedDelegate, float, CurrentValue, float, MaxValue);

UCLASS()
class BULLETANT_API ABACharacter : public ACharacter, public IAbilitySystemInterface, public IDataAssetInterface, public IFireStartInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ABACharacter();

protected:
    virtual void BeginPlay() override;
    
    virtual void PossessedBy(AController* NewController) override;

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

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
    class UInputAction* AttackAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
    class UInputAction* SwitchAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    class UInputAction* AimAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    class UInputAction* InteractionAction;

    // --- 실제 동작 함수 ---
protected:
    void Move(const FInputActionValue& Value);
    void Look(const FInputActionValue& Value);
    void Attack(const FInputActionValue& Value);
    void StartRunning(const FInputActionValue& Value);
    void StopRunning(const FInputActionValue& Value);
    void CrouchInput(const FInputActionValue& Value);
    void AimStart(const FInputActionValue& Value);
    void AimStop(const FInputActionValue& Value);
    void Interaction(const FInputActionValue& Value);
    void StartSwitchWeapon(const FInputActionValue& Value);

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

    void OnHealthChangedCallback(const FOnAttributeChangeData& Data) const;

#pragma endregion

#pragma region Weapon
public:
    virtual UDataAsset* GetDataAsset() const override;

    virtual FVector GetFireStartLocation() const override;
    virtual FVector GetFireDirection() const override;

    UFUNCTION()
    void EquipWeapon(TSubclassOf<ABaseWeapon> WeaponClass);

    UPROPERTY(EditDefaultsOnly, Category = "Combat|Weapon")
    TSubclassOf<ABaseWeapon> DefaultWeaponClass;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat|Weapon")
    TObjectPtr<ABaseWeapon> EquippedWeapon;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Weapon")
    TArray<TSubclassOf<ABaseWeapon>> OwnedEquipment;

#pragma endregion
    
};
