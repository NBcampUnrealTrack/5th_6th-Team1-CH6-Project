// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "SpawnManagerSubsystem.generated.h"

UCLASS()
class BULLETANT_API USpawnManagerSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
	
public:
	 AActor* GetTargetActor() const;
	
protected:	
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	
	void SetSpawnDataTable();
	void StartWave();
	void SpawnEnemies(int32 InWaveIndex);
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<AActor> TargetActor;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UDataTable> SpawnDataTable;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
	FDataTableRowHandle EnemySpawnHandle;	
	
	int32 WaveIndex;
	int32 AliveEnemyCount;
	
	FTimerHandle SpawnTimer;
};
