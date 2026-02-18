// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"	
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
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

#pragma region Base
public:
	ABaseEnemyCharacter();

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
#pragma endregion

public:
	UFUNCTION()
	virtual void OnDeath() override;

	AActor* GetTargetActor() const;

	virtual UDataAsset* GetDataAsset() const override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<AActor> TargetActor;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	float AcceptanceRadius = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	float RotateThreshold = 10.f;

	UPROPERTY(BlueprintReadOnly, Replicated)
	uint8 bIsTurning : 1;

	UPROPERTY(BlueprintReadOnly, Replicated)
	uint8 bIsTurningLeft : 1;

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
