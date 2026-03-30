// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Enemy/BaseEnemy/BaseEnemyCharacter.h"
#include "BaseSpitterEnemy.generated.h"

class USpitterDataAsset;
class UNiagaraComponent;

UCLASS()
class BULLETANT_API ABaseSpitterEnemy : public ABaseEnemyCharacter
{
	GENERATED_BODY()

public:
	ABaseSpitterEnemy();
	virtual void BeginPlay() override;

	void StartSpit();
	void CheckContinousSpit();
	void StopSpit();

	virtual UDataAsset* GetDataAsset() const override;
	
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Spitter")
	TObjectPtr<USpitterDataAsset> SpitterDataAsset;

	UPROPERTY(EditDefaultsOnly, Category = "Spitter")
	TObjectPtr<UNiagaraComponent> NiagaraComp;

	FVector LastCapsuleLocation;
	FQuat LastCapsuleRotation;
	bool bIsFirstCheck = true;

	FTimerHandle DamageChecker;
};
