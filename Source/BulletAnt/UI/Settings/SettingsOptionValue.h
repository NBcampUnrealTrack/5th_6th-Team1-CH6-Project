// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "SettingsOptionValue.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class USettingsOptionValue : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class BULLETANT_API ISettingsOptionValue
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual float GetValue() const = 0;

	virtual void SetValue(float InValue) = 0;
};
