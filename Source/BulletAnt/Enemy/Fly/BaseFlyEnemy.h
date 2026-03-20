// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Enemy/BaseEnemy/BaseEnemyCharacter.h"
#include "BaseFlyEnemy.generated.h"

UCLASS()
class BULLETANT_API ABaseFlyEnemy : public ABaseEnemyCharacter
{
	GENERATED_BODY()

public:
	virtual void ApplyTribe() override;

	void SetDiveMode();
	void UnSetDiveMode();

	void SetFlySpeed(float InSpeed);

protected:
	ABaseFlyEnemy();

	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PossessedBy(AController* NewController) override;

	UFUNCTION()
	void OnRep_FlySpeed();

	UFUNCTION()
	void OnRep_Deceleration();

	UFUNCTION()
	void OnRep_AccelerationRate();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SetDiveMode();
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_UnSetDiveMode();

protected:
	UPROPERTY(Replicated, ReplicatedUsing = OnRep_FlySpeed)
	float FlySpeed;

	UPROPERTY(Replicated, ReplicatedUsing = OnRep_Deceleration)
	float Deceleration;

	UPROPERTY(Replicated, ReplicatedUsing = OnRep_AccelerationRate)
	float AccelerationRate;
};
