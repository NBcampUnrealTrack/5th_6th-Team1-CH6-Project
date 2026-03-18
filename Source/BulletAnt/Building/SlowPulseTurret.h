// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Building/BaseTurret.h"
#include "SlowPulseTurret.generated.h"

class UGameplayEffect;
class UPulseTurretDataAsset;
class ABaseEnemyCharacter;

UCLASS()
class BULLETANT_API ASlowPulseTurret : public ABaseTurret
{
	GENERATED_BODY()
	
public:
	ASlowPulseTurret();

protected:
	virtual bool CanStartAttack() const override;
	virtual float GetAttackInterval() const override;
	virtual void ExecuteAttack() override;

	void GatherPulseTargets(TArray<class ABaseEnemyCharacter*>& OutEnemies) const;
	void ApplyEffectToEnemy(ABaseEnemyCharacter* Enemy, TSubclassOf<UGameplayEffect> EffectClass, float Level = 1.f) const;
	void ApplyDamageToEnemy(ABaseEnemyCharacter* Enemy) const;

	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_PlayPulseFX();

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Turret|Data")
	TObjectPtr<UPulseTurretDataAsset> PulseTurretData;
};
