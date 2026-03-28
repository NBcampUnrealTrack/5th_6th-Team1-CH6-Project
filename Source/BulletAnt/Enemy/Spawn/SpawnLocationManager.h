// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SpawnLocationManager.generated.h"

class ATriggerBox;

USTRUCT(BlueprintType)
struct BULLETANT_API FSpawnBoxEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	TObjectPtr<ATriggerBox> SpawnBox;

	UPROPERTY(VisibleAnywhere)
	FVector Origin;

	UPROPERTY(VisibleAnywhere)
	FVector2D Extent;

	UPROPERTY(VisibleAnywhere)
	FRotator Direction;

	UPROPERTY(VisibleAnywhere)
	float Weight;
};

UCLASS()
class BULLETANT_API ASpawnLocationManager : public AActor
{
	GENERATED_BODY()

public:
	ASpawnLocationManager();

	FVector GetRandomSpawnLocation() const;

protected:
	virtual void BeginPlay() override;

	const FSpawnBoxEntry& GetRandomSpawnBox() const;

protected:
	UPROPERTY(EditAnywhere, Category = "Spawning")
	TArray<FSpawnBoxEntry> SpawnBoxes;

	float TotalWeight = 0.0f;
};
