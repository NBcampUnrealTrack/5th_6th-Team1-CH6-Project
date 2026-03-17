#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "WeaponDataAsset.generated.h"

class UGameplayEffect;

UENUM(BlueprintType)
enum class ERarity : uint8
{
	Common		UMETA(DisplayName = "Common"),
	Rare		UMETA(DisplayName = "Rare"),
	SuperRare	UMETA(DisplayName = "SuperRare")
};

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	Rifle	UMETA(DisplayName = "Rifle"),
	Melee	UMETA(DisplayName = "Melee"),
	Shotgun UMETA(DisplayName = "Shotgun"),
	Sniper	UMETA(DisplayName = "Sniper"),
	Mining  UMETA(DisplayName = "Mining"),
	Jetpack UMETA(DisplayName = "Jetpack")
};

UCLASS(BlueprintType)
class BULLETANT_API UWeaponDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	FText WeaponName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	ERarity Rarity = ERarity::Common;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	EWeaponType WeaponType;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|GAS")
	FGameplayTag WeaponTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|GAS")
	FGameplayTag HitEventTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	TObjectPtr<USoundBase> ActiveSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|GAS")
	TSubclassOf<UGameplayEffect> OnUseStateHitEffect;
};
