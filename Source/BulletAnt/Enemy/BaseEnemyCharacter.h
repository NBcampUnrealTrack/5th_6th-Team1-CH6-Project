// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"	
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "Weapon/Data/MeleeWeaponDataAsset.h"
#include "Common/DataAssetInterface.h"
#include "BaseEnemyCharacter.generated.h"

class UStateTreeComponent;
class UBaseEnemyDataAsset;

UCLASS()
class BULLETANT_API ABaseEnemyCharacter : public ACharacter, public IAbilitySystemInterface, public IDataAssetInterface
{
	GENERATED_BODY()

public:
	ABaseEnemyCharacter();
	
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	UStateTreeComponent* GetStateTreeComponent() const;
	AActor* GetTargetActor() const;

protected:
	virtual void BeginPlay() override;
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config")
	TObjectPtr<UBaseEnemyDataAsset> BaseEnemyDataAsset;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<AActor> TargetActor;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	float AcceptanceRadius;
	
	virtual UDataAsset* GetDataAsset() const override;
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UStateTreeComponent> StateTreeComponent;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config")
	TObjectPtr<UWeaponDataAsset> BaseEnemyAttackDataAsset;
};
