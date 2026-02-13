#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "AbilitySystemInterface.h"
#include "Common/DataAssetInterface.h"
#include "Common/FireStartInterface.h"
#include "GameplayEffectTypes.h"
#include "Net/UnrealNetwork.h"
#include "BACharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UMotionWarpingComponent;
class UInputAction;
class UHealthAttributeSet;
class ABaseWeapon;
class UBuildManagerComponent;
class UBAParkourComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAttributeChangedDelegate, float, CurrentValue, float, MaxValue);

UENUM(BlueprintType)
enum class ETurnType : uint8
{
    None,
    Left90,
    Right90,
    Left180,
    Right180
};
enum class EAbilityInputID : uint8
{
    None,
    Fire,
    Reload,
    Aim
};

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
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    virtual void OnRep_Controller() override;


    FORCEINLINE TObjectPtr<USkeletalMeshComponent> GetFPSMesh() { return FPSMesh; } const
    //TEST
    FORCEINLINE UCameraComponent* GetCamera() const { return FirstPersonCameraComponent; }


protected:
    // --- 카메라 관련 컴포넌트 ---
    //UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
    //TObjectPtr<USpringArmComponent> CameraBoom;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
    TObjectPtr<UCameraComponent> FirstPersonCameraComponent;
    // --- 1인칭 관련 컴포넌트 ---
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "FPS")
    TObjectPtr<USkeletalMeshComponent> FPSMesh;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Motion Warping")
    TObjectPtr<UMotionWarpingComponent> MotionWarpingComp;


#pragma region InputAction

public:
    // --- 입력(Enhanced Input) 관련 변수 ---
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* JumpAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* MoveAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* RunAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* CrouchAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* LookAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    class UInputAction* AttackAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    class UInputAction* SwitchAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* AimAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* InteractionAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Building")
    UInputAction* EnterBuildModeAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Building")
    UInputAction* ExitBuildModeAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Building")
    UInputAction* PlaceBuildingAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Building")
    UInputAction* RotateBuildingAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Building")
    UInputAction* ToggleSnapModeAction;

#pragma endregion

#pragma region Action Function

    // --- 실제 동작 함수 ---
protected:
    void Move(const FInputActionValue& Value);
    void Look(const FInputActionValue& Value);
    void StartAttack(const FInputActionValue& Value);
    void StopAttack(const FInputActionValue& Value);
    void StartRunning(const FInputActionValue& Value);
    void StopRunning(const FInputActionValue& Value);
    void CrouchInput(const FInputActionValue& Value);
    void AimStart(const FInputActionValue& Value);
    void AimStop(const FInputActionValue& Value);
    void Interaction(const FInputActionValue& Value);
    void EnterBuildMode(const FInputActionValue& Value);
    void ExitBuildMode(const FInputActionValue& Value);
    void PlaceBuilding(const FInputActionValue& Value);
    void RotateBuilding(const FInputActionValue& Value);
    void ToggleSnapMode(const FInputActionValue& Value);
    void StartSwitchWeapon(const FInputActionValue& Value);
    void JumpHandler(const FInputActionValue& Value);


public:

#pragma endregion

#pragma region Farkour
protected:
    UFUNCTION()
    void OnTurnMontageEnded(UAnimMontage* Montage, bool bInterrupted);

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Parkour", meta = (AllowPrivateAccess = "true"))
    UBAParkourComponent* ParkourComponent;



#pragma endregion

#pragma region Animation
public:
    // 상태에 따른 이동속도
    float UpdateMovementSpeed();
    void IdleTurning(float DeltaTime);
    void SetTurnStatus();
protected:
    UFUNCTION(NetMulticast, Reliable)
    void Multicast_PlayTurnMontage(UAnimMontage* MontageToPlay, FTransform TargetTransform);
    UFUNCTION(Server, Reliable)
    void ServerRPC_StopTurnMontage();
    UFUNCTION(NetMulticast, Reliable)
    void Multicast_StopTurnMontage();

public:

    //조준상태
    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Input")
    bool bIsAiming;
    UFUNCTION(Server, Reliable)
    void Server_SetAiming(bool bNewIsAiming);

    //달리기 상태
    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Input")
    bool bIsRunning;
    UFUNCTION(Server, Reliable)
    void Server_SetRunning(bool bNewIsRunning);

    // --- 에디터 수정 가능 ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float WalkSpeed = 400.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float AimSpeed = 350.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float RunningSpeed = 800.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float CrouchSpeed = 300.f;


    //최대 조준시 좌우 시선 회전 각도
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CharAnimation")
    float AimTurn = 45.f;
    //최대 좌우 시선 회전 각도
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CharAnimation")
    float IdleTurn = 90.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CharAnimation")
    float LineTraceRange = 500.f;
    UPROPERTY(Replicated)
    float SyncAimYaw;
    UPROPERTY(Replicated)
    float SyncAimPitch;

    float RemoteViewYaw;
    FRotator ControlRot;
    float CurrentTurnSpeed;
    FRotator DeltaRot;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Turn")
    TObjectPtr<UAnimMontage> TurnLeft90Montage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Turn")
    TObjectPtr<UAnimMontage> TurnRight90Montage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Turn")
    TObjectPtr<UAnimMontage> TurnLeft180Montage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Turn")
    TObjectPtr<UAnimMontage> TurnRight180Montage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Turn")
    TObjectPtr<UAnimMontage> CrouchTurnLeft90Montage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Turn")
    TObjectPtr<UAnimMontage> CrouchTurnRight90Montage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Turn")
    TObjectPtr<UAnimMontage> CrouchTurnLeft180Montage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Turn")
    TObjectPtr<UAnimMontage> CrouchTurnRight180Montage;

    UAnimMontage* CurrentTurnMontage;

public:
    UPROPERTY(Replicated)
    bool bIsTurning;
    float TurnDelayTimer;
    UPROPERTY(Replicated, BlueprintReadOnly)
    ETurnType TurnType;
#pragma endregion

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



    UFUNCTION(Server,Reliable)
    void Server_EquipWeapon(TSubclassOf<ABaseWeapon> WeaponClass);

    UPROPERTY(EditDefaultsOnly, Category = "Combat|Weapon")
    TSubclassOf<ABaseWeapon> DefaultWeaponClass;

    UPROPERTY(VisibleAnywhere,Replicated, BlueprintReadWrite, Category = "Combat|Weapon")
    TObjectPtr<ABaseWeapon> EquippedWeapon;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Weapon")
    TArray<TSubclassOf<ABaseWeapon>> OwnedEquipment;

#pragma endregion
    
protected:
    UPROPERTY(VisibleAnywhere)
    UBuildManagerComponent* BuildManager;
   
};
