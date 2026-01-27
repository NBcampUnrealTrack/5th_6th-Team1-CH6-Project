#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "GameplayAbilitySpec.h"
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
	void EquipWeapon(UAbilitySystemComponent* ASC);
	void UnequipWeapon(UAbilitySystemComponent* ASC);

	FORCEINLINE USkeletalMeshComponent* GetWeaponMesh() const { return WeaponMesh; }
	FORCEINLINE UWeaponDataAsset* GetWeaponData() const { return WeaponData; }
	FORCEINLINE FGameplayTag GetWeaponTag() const { return WeaponTag; }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	TObjectPtr<USkeletalMeshComponent> WeaponMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|GAS")
	FGameplayTag WeaponTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|GAS")
	TArray<TSubclassOf<UGameplayAbility>> GrantedAbilities;

	UPROPERTY()
	TArray<FGameplayAbilitySpecHandle> GrantedAbilityHandles;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Data")
	TObjectPtr<UWeaponDataAsset> WeaponData;
};
