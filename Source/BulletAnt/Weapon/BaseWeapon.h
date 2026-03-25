#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "GameplayAbilitySpec.h"
#include "GameplayEffect.h"
#include "BaseWeapon.generated.h"

class USkeletalMeshComponent;
class UAbilitySystemComponent;
class UGameplayAbility;
class UWeaponDataAsset;

UCLASS(Abstract)
class BULLETANT_API ABaseWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	ABaseWeapon();

public:
	FORCEINLINE UWeaponDataAsset* GetWeaponData() const { return WeaponData; }
	FORCEINLINE USkeletalMeshComponent* GetWeaponMesh() const{ return WeaponMesh; }
	FORCEINLINE TArray<TSubclassOf<UGameplayAbility>> GetGrantedAbilities() const { return GrantedAbilities; }
	FORCEINLINE TArray<FGameplayAbilitySpecHandle> GetGrantedAbilityHandles() const { return GrantedAbilityHandles; }
	FORCEINLINE TSubclassOf<UGameplayEffect> GetSwitchEffectClass() const { return SwitchEffectClass; }

	virtual void EquipWeapon(UAbilitySystemComponent* ASC);
	void UnequipWeapon(UAbilitySystemComponent* ASC);

	void SetStencilValue(int32 NewValue);

	UPROPERTY()
	uint8 bAutoActive : 1 = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Settings")
	FTransform GripOffset;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Root")
	TObjectPtr<USceneComponent> RootComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	TObjectPtr<USkeletalMeshComponent> WeaponMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|GAS")
	TArray<TSubclassOf<UGameplayAbility>> GrantedAbilities;

	UPROPERTY()
	TArray<FGameplayAbilitySpecHandle> GrantedAbilityHandles;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Data")
	TObjectPtr<UWeaponDataAsset> WeaponData;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon|GAS")
	TSubclassOf<UGameplayEffect> SwitchEffectClass;

	
	
};
