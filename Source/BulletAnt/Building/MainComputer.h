// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Building/BaseBuilding.h"
#include "MainComputer.generated.h"

/**
 * 
 */
UCLASS()
class BULLETANT_API AMainComputer : public ABaseBuilding
{
	GENERATED_BODY()

public:
	virtual void GetInteractionOptions_Implementation(AActor* User, TArray<FInteractionOption>& OutOptions) const;
	virtual void Interaction_Implementation(AActor* User, FName ActionName);
};
