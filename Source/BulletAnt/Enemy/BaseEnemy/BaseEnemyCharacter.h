// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"	
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "Common/DataAssetInterface.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Enemy/DataAsset/TargetPriority.h"
#include "Net/UnrealNetwork.h"
#include "BaseEnemyCharacter.generated.h"

class UStateTreeComponent;
class UBaseEnemyDataAsset;
class UHealthAttributeSet;
class USphereComponent;
class UTribeDataAsset;

USTRUCT()
struct FActorArrayWrapper
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<TObjectPtr<AActor>> Actors;
};

UCLASS()
class BULLETANT_API ABaseEnemyCharacter : public ACharacter, public IAbilitySystemInterface, public IDataAssetInterface
{
	GENERATED_BODY()

#pragma region Base
public:
	ABaseEnemyCharacter();

protected:
	virtual void BeginPlay() override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
#pragma endregion

public:
	AActor* GetTargetActor() const;

	virtual UDataAsset* GetDataAsset() const override;

	virtual bool ShouldCallAfterAttack();

	virtual void AfterAttack();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SetNoCollision();

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

#pragma region Perception

public:
	USphereComponent* GetDetectionSphere() const;

protected:
	UFUNCTION()
	virtual void OnDetectionSphereBeginOverlap(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	UFUNCTION()
	virtual void OnDetectionSphereEndOverlap(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex);

	void SenseNearbyActors();

	bool IsInFieldOfView(AActor* Target, float FOVAngle);

protected:
	UPROPERTY(VisibleAnywhere, Category = "Perception")
	TObjectPtr<USphereComponent> DetectionSphere;

	ETargetPriorityType TargetActorPriority;

	UPROPERTY(VisibleAnywhere, Category = "Perception")
	TMap<ETargetPriorityType, FActorArrayWrapper> NearbyActors;


	FTimerHandle SensingTimerHandle;

#pragma endregion

#pragma region GAS

public:
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

protected:
	void InitGAS();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY()
	UHealthAttributeSet* HealthAttributeSet;

	FDelegateHandle DeadEventHandle;

	//FActiveGameplayEffectHandle DeathGEHandle;


#pragma endregion

#pragma region StateTree

public:
	UStateTreeComponent* GetStateTreeComponent() const;

protected:
	void OnDeadEventReceived(const FGameplayEventData* Payload);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UStateTreeComponent> StateTreeComponent;

#pragma endregion

#pragma region DataAsset

public:
	UTribeDataAsset* GetTribeType() const;

	void SetTribeType(UTribeDataAsset* InTribeType);

	void ApplyTribe();

	void ApplyTribeMaterial();

	void ApplyTribePriority();

	UAnimMontage* GetDieAnimMontage() const;

	//void Die();

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config")
	TObjectPtr<UBaseEnemyDataAsset> BaseEnemyDataAsset;

protected:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config")
	TObjectPtr<UTribeDataAsset> TribeType;

#pragma endregion

#pragma region Init

public:
	float GetWalkSpeed() const;
	void SetWalkSpeed(float InWalkSpeed);

protected:
	UPROPERTY(Replicated, ReplicatedUsing = OnRep_WalkSpeed)
	float WalkSpeed;

	UFUNCTION()
	void OnRep_WalkSpeed();

#pragma endregion

};
