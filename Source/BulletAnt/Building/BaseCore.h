// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Building/BaseBuilding.h"
#include "BaseCore.generated.h"

/**
 * 
 */
UCLASS()
class BULLETANT_API ABaseCore : public ABaseBuilding
{
	GENERATED_BODY()
	
protected:
	virtual void BeginPlay() override;

};
