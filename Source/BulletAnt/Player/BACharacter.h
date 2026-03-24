#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "Common/DataAssetInterface.h"
#include "Common/FireStartInterface.h"
#include "Common/OnDeathInterface.h"
#include "AbilitySystemInterface.h"
#include "GameplayEffectTypes.h"
#include "Net/UnrealNetwork.h"
#include "Engine/DataTable.h"
#include "BACharacter.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHealthAttributeChangedDelegate, float, CurrentValue, float, MaxValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAmmoChangedDelegate, float, CurrentAmmoValue, float, MaxAmmoValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEXPChangedDelegate, float, CurrentEXPValue, float, MaxEXPValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLevelChangedDelegate, float, CurrentLevel, float, OldLevel);


class UCapsuleComponent;
class USpringArmComponent;
class UCameraComponent;
class ABAPlayerController;
class UMotionWarpingComponent;
class UInputAction;
class UHealthAttributeSet;
class UAmmoAttributeSet;
class UEXPAttributeSet;
class ABaseWeapon;
class UBuildManagerComponent;
class UBAParkourComponent;
class USceneCaptureComponent2D;
class UUISubsystem;
class UGameplayEffect;
class USplineComponent;
class UNiagaraComponent;

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

UENUM(BlueprintType)
enum class EEquipmentType : uint8
{
    Ranged       UMETA(DisplayName = "Ranged"),
    Mining      UMETA(DisplayName = "Mining"),
    Melee       UMETA(DisplayName = "Melee")
};
USTRUCT(BlueprintType)
struct FAnimChoice : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere)
    UAnimSequence* AnimAsset;

    UPROPERTY(EditAnywhere)
    float TargetAngle;
    
    UPROPERTY(EditAnywhere)
    float TargetSpeed;
};

UCLASS()
class BULLETANT_API ABACharacter : public ACharacter, public IDataAssetInterface, public IFireStartInterface, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ABACharacter();


protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void PossessedBy(AController* NewController) override;

public:
    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    virtual void OnRep_Controller() override;
    virtual void OnRep_PlayerState() override;


    //TEST
    FORCEINLINE UCameraComponent* GetCamera() const { return CameraComponent; }
    FORCEINLINE USpringArmComponent* GetSpringArm() const { return SpringArm; };
    void SpringArmRot(bool check);

protected:
    // --- 카메라 관련 컴포넌트 ---

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DetectedCapsule")
    TObjectPtr < UCapsuleComponent> DetectedCapsule;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spring Arm")
    TObjectPtr < USpringArmComponent> SpringArm;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
    TObjectPtr<UCameraComponent> CameraComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Motion Warping")
    TObjectPtr<UMotionWarpingComponent> MotionWarpingComp;

    ABAPlayerController* PC;

private:
    UPROPERTY()
    TArray<UPrimitiveComponent*> HiddenComp;


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
    UInputAction* ReloadAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* AttackAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* SwitchAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* AimAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* ADSAction;

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

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Building")
    UInputAction* SelectCat1Action;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Building")
    UInputAction* SelectCat2Action;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Building")
    UInputAction* SelectCat3Action;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Building")
    UInputAction* CyclePrevAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Building")
    UInputAction* CycleNextAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Building")
    UInputAction* ToggleBuildInfoAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|GroundScanner")
    UInputAction* GroundScannerAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Communicate")
    UInputAction* PingAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|GroundReturner")
    UInputAction* ReturnAction;

#pragma endregion

#pragma region Action Function

    // --- 실제 동작 함수 ---
public:

protected:
    void Move(const FInputActionValue& Value);
    void Look(const FInputActionValue& Value);
    void StartAttack(const FInputActionValue& Value);
    void Reload(const FInputActionValue& Value);
    void AimStart(const FInputActionValue& Value);
    void AimStop(const FInputActionValue& Value);
    void StopAttack(const FInputActionValue& Value);
    void StartRunning(const FInputActionValue& Value);
    void StopRunning(const FInputActionValue& Value);
    void CrouchInput(const FInputActionValue& Value); 
    void ADSStart(const FInputActionValue& Value);
    void Interaction(const FInputActionValue& Value);
    void EnterBuildMode(const FInputActionValue& Value);
    void ExitBuildMode(const FInputActionValue& Value);
    void PlaceBuilding(const FInputActionValue& Value);
    void RotateBuilding(const FInputActionValue& Value);
    void ToggleSnapMode(const FInputActionValue& Value);
    void OnSelectCat1(const FInputActionValue& Value);
    void OnSelectCat2(const FInputActionValue& Value);
    void OnSelectCat3(const FInputActionValue& Value);
    void OnCyclePrev(const FInputActionValue& Value);
    void OnCycleNext(const FInputActionValue& Value);
    void OnToggleBuildInfo(const FInputActionValue& Value);
    void StartSwitchWeapon(const FInputActionValue& Value);
    void JumpHandler(const FInputActionValue& Value);
    void ExecutePing(const FInputActionValue& Value);
    void SwitchReturnMode(const FInputActionValue& Value);


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
    void StopMontage();
protected:
    UFUNCTION(NetMulticast, Reliable)
    void Multicast_PlayTurnMontage(UAnimMontage* MontageToPlay, FTransform TargetTransform);
    UFUNCTION(Server, Reliable)
    void ServerRPC_StopTurnMontage();
    UFUNCTION(NetMulticast, Reliable)
    void Multicast_StopTurnMontage();


    UFUNCTION(Server, Reliable)
    void Server_SetAiming(bool bNewIsAiming);

    UFUNCTION(Server, Reliable)
    void Server_SetRunning(bool bNewIsRunning);
public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anim Data")
    TArray<FAnimChoice> AnimDataBase;

    //조준상태
    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Input")
    bool bIsAiming;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Input")
    bool bIsADS;

    UPROPERTY(BlueprintReadWrite, Category = "Camera")
    float AimingFieldOfView = 80.f;
    UPROPERTY(BlueprintReadWrite, Category = "SpringArm")
    float AimingTALength = 100.f;
    UPROPERTY(BlueprintReadWrite, Category = "SpringArm")
    float TALengthChangeSpeed = 15.f;

    //달리기 상태
    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Input")
    bool bIsRunning;
    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    EEquipmentType CurrentEquipmentType = EEquipmentType::Ranged;

    // --- 에디터 수정 가능 ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float WalkSpeed = 400.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float AimSpeed = 350.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float RunningSpeed = 800.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float CrouchSpeed = 300.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float SpringArmZ;


    //최대 조준시 좌우 시선 회전 각도
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CharAnimation")
    float AimTurn = 45.f;
    //최대 좌우 시선 회전 각도
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CharAnimation")
    float IdleTurn = 90.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    float LineTraceRange = 500.f;
    UPROPERTY(Replicated)
    float SyncAimYaw;
    UPROPERTY(Replicated)
    float SyncAimPitch;

    float RemoteViewYaw;
    FRotator ControlRot;
    float CurrentTurnSpeed;

    float LastBodyYaw;
    float RootYawOffset;
    float TurnStartYaw;

    float DefaultArmLength = 233.0f;

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
    UPROPERTY(BlueprintAssignable, Category = "Combat|UI")
    FOnHealthAttributeChangedDelegate OnHealthChanged;

    UPROPERTY(BlueprintAssignable, Category = "Combat|UI")
    FOnAmmoChangedDelegate OnAmmoChanged;

    UPROPERTY(BlueprintAssignable, Category = "Combat|UI")
    FOnEXPChangedDelegate OnEXPChanged;

    UPROPERTY(BlueprintAssignable, Category = "Combat|UI")
    FOnLevelChangedDelegate OnLevelChanged;

    virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

    void ShowAmmo();
    void HideAmmo();

    void GetEXP(float InEXP);
    void LevelUp();

protected:
    void OnHealthChangedCallback(const FOnAttributeChangeData& Data) const;

    void OnAmmoChangedCallback(const FOnAttributeChangeData& Data) const;

    void OnEXPChangedCallback(const FOnAttributeChangeData& Data) const;

    void OnLevelChangedCallback(const FOnAttributeChangeData& Data);

    UAbilitySystemComponent* ASC;

    UHealthAttributeSet* HealthAttributeSet;
    const UAmmoAttributeSet* AmmoAttributeSet;
    const UEXPAttributeSet* EXPAttributeSet;

    UPROPERTY(EditDefaultsOnly, Category = "Effect")
    TSubclassOf<UGameplayEffect> EXPEffectClass;

    UPROPERTY(EditDefaultsOnly, Category = "Effect")
    TSubclassOf<UGameplayEffect> LevelUpEffectClass;

#pragma endregion

#pragma region Weapon
public:
    virtual UDataAsset* GetDataAsset() const override;

    virtual FVector GetFireStartLocation_Implementation() const override;
    virtual FVector GetFireDirection_Implementation() const override;

    void StartAiming();
    void EndAiming();

    UFUNCTION()
    void OnRep_EquippedWeapon();

    UFUNCTION(Server,Reliable)
    void Server_SetChangeWeapon(TSubclassOf<ABaseWeapon> InWeapon, int32 WeaponIndex = 0);

    UFUNCTION()
    void OnRep_bIsFiring();

    UFUNCTION(Server,Reliable)
    void Server_EquipWeapon(TSubclassOf<ABaseWeapon> WeaponClass);

    void SetbIsFiring(bool InIsFiring);

    UFUNCTION()
    void RequestWeaponLog(UWeaponDataAsset* InData);

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_ShowWeaponLog(UWeaponDataAsset* InData);

    UPROPERTY(EditDefaultsOnly, Category = "Combat|Weapon")
    TSubclassOf<ABaseWeapon> DefaultWeaponClass;

    UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_EquippedWeapon, BlueprintReadWrite, Category = "Combat|Weapon")
    TObjectPtr<ABaseWeapon> EquippedWeapon;

    UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Combat|Weapon")
    TArray<TSubclassOf<ABaseWeapon>> OwnedEquipment;

    FTransform SavedSpringArmTransform;
    
    UPROPERTY(ReplicatedUsing = OnRep_bIsFiring)
    bool bIsFiring = false;

#pragma endregion

#pragma region Recoil
public:
    void SetRecoil(float InPitch, float InYaw);

protected:
    float CurrentRecoilPitch = 0.f;
    float CurrentRecoilYaw = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, category = "Recoil")
    float RecoilInterpSpeed = 10.f;
#pragma endregion

protected:

    UFUNCTION()
    void HandleRespawnUI(FGameplayTag Tag, int32 NewCount);

    UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category = "Combat|Respawn")
    float RespawnTime = 5.f;

protected:
    UPROPERTY(VisibleAnywhere)
    UBuildManagerComponent* BuildManager;

#pragma region GroundScanner

public:
    FORCEINLINE USceneCaptureComponent2D* GetGroundScannerSceneCapture() { return SceneCapture2D; }
    void RotateScannerParent(const FVector2D& Input);
    void ChangeScannerDistance(float Input);
    void SwitchGroundScanner();

    void InitializeSceneCapture();
    void UpdateShowComponents();
    
    void SetPlayerColor();
        
protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GroundScanner")
    TObjectPtr<USpringArmComponent> SceneCaptureParent;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GroundScanner")
    TObjectPtr<USceneCaptureComponent2D> SceneCapture2D;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GroundScanner")
    TObjectPtr<UMaterialInterface> M_PostProcessGroundScanner;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GroundScanner")
    TObjectPtr<UTextureRenderTarget2D> RT_GroundScanner;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GroundScanner")
    float DefaultScannerDistance = 600.0f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GroundScanner")
    float MinScannerDistance = 300.0f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GroundScanner")
    float MaxScannerDistance = 900.0f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GroundScanner")
    float ScannerZoomMultiplier = 60.0f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GroundScanner")
    FRotator ScannerDefaultRotation = FRotator(-40.0f, 0.0f, 0.0f);
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GroundScanner")
    float ScannerRotateMultiplier = 0.2f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GroundScanner")
    TObjectPtr<UStaticMeshComponent> ArrowMesh;

    FDelegateHandle PlayerColorChangeHandle;

    static TWeakObjectPtr<USceneCaptureComponent2D> LocalSceneCapture;      // 해당 클라이언트에서 제어 중인 플레이어의 SceneCapture2D 

#pragma endregion

#pragma region GroundReturner

public:
    UFUNCTION(Server, Reliable)
    void Server_ResetPath();
    UFUNCTION(Server, Reliable)
    void Server_StartRecordingPath();
    UFUNCTION(Server, Reliable)
    void Server_StopRecordingPath();

    FORCEINLINE bool GetIsReturning() const { return bIsReturning; }

protected:
    void ResetPath();
    void StartRecordingPath();
    void StopRecordingPath();
    void UpdateSplinePath();

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_ResetPath();

    void AddPathPoint(FVector NewPoint);
    void RemovePathPoints(int32 LastIdx);
    int32 GetRemainedPathIdx(float FinalDistance);

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_AddPathPoint(FVector NewPoint);
    UFUNCTION(NetMulticast, Reliable)
    void Multicast_RemovePoints(int32 LastIdx);

    void SetIsReturning(bool bInReturning);
    void StartReturning();
    void HandleReturnMovement(float DeltaTime);
    void StopReturning();
    UFUNCTION(Server, Reliable)
    void Server_StartReturning();
    UFUNCTION(Server, Reliable)
    void Server_StopReturning();

    void ActivateReturnEffect();
    void DeactivateReturnEffect();

    UFUNCTION()
    void OnRep_IsReturning();

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GroundReturner")
    TObjectPtr<USplineComponent> PathSpline;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GroundReturner")
    TObjectPtr<UNiagaraComponent> ReturnEffect;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GroundReturner")
    TObjectPtr<UNiagaraComponent> ReturnPathEffect;

    FTimerHandle PathUpdateTimer;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GroundReturner")
    float PathDistThreshold = 50.0f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GroundReturner")
    float ReturnSpeed = 1500.0f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GroundReturner")
    float ReturnArmLength = 50.0f;
    
    float ReturnDistance = 0.0f;

    UPROPERTY(ReplicatedUsing = OnRep_IsReturning)
    uint8 bIsReturning : 1 = false;

    static const FName NameReturnEffectColor;
    static const FName NameReturnPathEffectColor;
    static const FName NameReturnPathEffectSpawnRate;

#pragma endregion

};
