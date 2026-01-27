// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "GameplayTagContainer.h"
#include "InputActionValue.h"
#include "BACharacter.generated.h"

class UAbilitySystemComponent;
class UBAAttributeSet;
class ABaseWeapon;

UCLASS()
class BULLETANT_API ABACharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	ABACharacter();

	// --- IAbilitySystemInterface ---
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

protected:
    virtual void BeginPlay() override;
    virtual void PossessedBy(AController* NewController) override;

public:
    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// --- GAS Components ---
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	TObjectPtr<UBAAttributeSet> AttributeSet;

	void InitializeAttributes();

	// --- Weapon ---
public:
	void EquipWeapon(ABaseWeapon* NewWeapon);
	void UnequipWeapon();

	FORCEINLINE ABaseWeapon* GetCurrentWeapon() const { return CurrentWeapon; }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	TObjectPtr<ABaseWeapon> CurrentWeapon;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	TSubclassOf<ABaseWeapon> DefaultWeaponClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	FName WeaponSocketName = TEXT("weapon_r");

    // --- Camera Components ---
protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
    class USpringArmComponent* CameraBoom;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
    class UCameraComponent* FollowCamera;

    // --- Input (Enhanced Input) ---
public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
    class UInputAction* JumpAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
    class UInputAction* MoveAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
    class UInputAction* LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	class UInputAction* FireAction;

	// Fire Ability Tag (Set in Editor)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS")
	FGameplayTag FireAbilityTag;

    // --- Input Handler Functions ---
protected:
    void Move(const FInputActionValue& Value);
    void Look(const FInputActionValue& Value);
	void StartFire(const FInputActionValue& Value);
	void StopFire(const FInputActionValue& Value);
};
