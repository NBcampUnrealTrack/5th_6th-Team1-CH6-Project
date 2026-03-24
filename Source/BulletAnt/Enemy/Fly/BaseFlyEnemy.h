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

	float GetFlySpeed() const;
	void SetFlySpeed(float InSpeed);

protected:
	ABaseFlyEnemy();

	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PossessedBy(AController* NewController) override;

	virtual void OnDetectionSphereBeginOverlap(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult) override;

	virtual void OnDetectionSphereEndOverlap(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex) override;

	virtual void OnMoveAttributeChange(const FOnAttributeChangeData& Data) override;

	UFUNCTION()
	void OnRep_Deceleration();

	UFUNCTION()
	void OnRep_AccelerationRate();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SetDiveMode();
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_UnSetDiveMode();

protected:
	UPROPERTY(Replicated, ReplicatedUsing = OnRep_Deceleration)
	float Deceleration;

	UPROPERTY(Replicated, ReplicatedUsing = OnRep_AccelerationRate)
	float AccelerationRate;
};
