// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "WeaponDataAsset.generated.h"

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	Rifle	UMETA(DisplayName = "Rifle"),
	Melee	UMETA(DisplayName = "Melee"),
	Shotgun UMETA(DisplayName = "Shotgun"),
	Sniper	UMETA(DisplayName = "Sniper"),
	Mining  UMETA(DisplayName = "Mining")
};

UCLASS()
class BULLETANT_API UWeaponDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	EWeaponType WeaponType;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	FGameplayTag WeaponTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	FGameplayTag HitEventTag;
};
