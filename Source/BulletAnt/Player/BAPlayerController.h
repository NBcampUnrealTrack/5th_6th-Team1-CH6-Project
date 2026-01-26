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
	// --- [블루프린트 입력용 변수]
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputMappingContext* DefaultMappingContext;

protected:
	virtual void BeginPlay() override;
	
};
