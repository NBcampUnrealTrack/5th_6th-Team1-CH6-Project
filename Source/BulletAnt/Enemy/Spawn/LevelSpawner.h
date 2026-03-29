// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LevelSpawner.generated.h"

class UBoxComponent;
class ULevelSpanwerDataAsset;
class ABaseEnemyCharacter;
class UTribeDataAsset;
class ABACharacter;

UCLASS()
class BULLETANT_API ALevelSpawner : public AActor
{
	GENERATED_BODY()
	
public:	
	ALevelSpawner();

	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

protected:
	UFUNCTION()
	virtual void OnDetectionBoxBeginOverlap(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	UFUNCTION()
	virtual void OnDetectionBoxEndOverlap(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex);

	void SpawnEnemy();

	FVector GetRandomLocation();

	TSubclassOf<ABaseEnemyCharacter> SelectEnemyClass() const;

	UTribeDataAsset* SelectTribe() const;

	ABACharacter* SelectTarget() const;

protected:
	UPROPERTY(VisibleAnywhere, Category = "Spawner")
	TObjectPtr<UBoxComponent> DetectionBox;

	UPROPERTY(VisibleAnywhere, Category = "Spawner")
	TObjectPtr<UBoxComponent> SpawnBox;

	UPROPERTY(EditAnywhere, Category = "Spawner")
	TObjectPtr<ULevelSpanwerDataAsset> SpawnDataAsset;

	UPROPERTY(VisibleAnywhere, Category = "Spawner")
	TArray<TWeakObjectPtr<ABACharacter>> TargetActors;

	UPROPERTY(EditAnywhere, Category = "Spawner")
	int32 EnemyCount = 10;

	bool bAlreadyActivated = false;

	FTimerHandle SpawnIntervalTimerHandle;
};
