#pragma once

#include "CoreMinimal.h"
#include "Weapon/Data/WeaponDataAsset.h"
#include "GameplayTagContainer.h"
#include "MeleeWeaponDataAsset.generated.h"

UCLASS()
class BULLETANT_API UMeleeWeaponDataAsset : public UWeaponDataAsset
{
	GENERATED_BODY()

public:

	UPROPERTY(EditDefaultsOnly)
	float BaseDamage;

	UPROPERTY(EditDefaultsOnly)
	float AttackRadius;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UAnimMontage> AttackMontage;

	UPROPERTY(EditAnywhere, Category = "Attack")
	FName SocketName;
	
};
