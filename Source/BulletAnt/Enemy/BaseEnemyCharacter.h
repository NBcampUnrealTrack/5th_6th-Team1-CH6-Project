// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"	
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "Weapon/Data/MeleeWeaponDataAsset.h"
#include "Common/DataAssetInterface.h"
#include "Common/OnDeathInterface.h"
#include "BaseEnemyCharacter.generated.h"

class UStateTreeComponent;
class UBaseEnemyDataAsset;
class UHealthAttributeSet;

UCLASS()
class BULLETANT_API ABaseEnemyCharacter : public ACharacter, public IAbilitySystemInterface, public IDataAssetInterface, public IOnDeathInterface
{
	GENERATED_BODY()

public:
	UFUNCTION()
	virtual void OnDeath() override;
	
	ABaseEnemyCharacter();
	
	AActor* GetTargetActor() const;

protected:
	virtual void BeginPlay() override;
	
public:	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<AActor> TargetActor;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	float AcceptanceRadius;
	
	virtual UDataAsset* GetDataAsset() const override;
	
protected:	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config")
	TObjectPtr<UWeaponDataAsset> BaseEnemyAttackDataAsset;
	
#pragma region GAS
	
public:
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;
	
	UPROPERTY()
	UHealthAttributeSet* HealthAttributeSet;
	
#pragma endregion
	
#pragma region StateTree
	
public:
	UStateTreeComponent* GetStateTreeComponent() const;
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UStateTreeComponent> StateTreeComponent;

#pragma endregion
	
#pragma region BaseEnemyDataAsset
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config")
	TObjectPtr<UBaseEnemyDataAsset> BaseEnemyDataAsset;
	
#pragma endregion

};
