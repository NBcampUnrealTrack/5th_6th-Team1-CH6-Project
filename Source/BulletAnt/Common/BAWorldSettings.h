// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/WorldSettings.h"
#include "BAWorldSettings.generated.h"

/**
 * 
 */
UCLASS()
class BULLETANT_API ABAWorldSettings : public AWorldSettings
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category="Spawn")
	UDataTable* SpawnTable;
};
