// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "DataAssetInterface.generated.h"

class UWeaponDataAsset;

UINTERFACE(MinimalAPI)
class UDataAssetInterface : public UInterface
{
	GENERATED_BODY()
};

class BULLETANT_API IDataAssetInterface
{
	GENERATED_BODY()

public:
	virtual UDataAsset* GetDataAsset() const = 0;
};
