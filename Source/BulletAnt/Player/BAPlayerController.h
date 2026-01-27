// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BAPlayerController.generated.h"

/**
 * 
 */
class UInputMappingContext;
UCLASS()
class BULLETANT_API ABAPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	// --- Input Variables for Blueprint ---
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputMappingContext* DefaultMappingContext;

protected:
	virtual void BeginPlay() override;
	
};
