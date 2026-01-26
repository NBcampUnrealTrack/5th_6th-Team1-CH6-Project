// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityConfigAsset.h"
#include "BAMassEntityConfigAsset.generated.h"

class UMassStateTreeTrait;

UCLASS()
class BULLETANT_API UBAMassEntityConfigAsset : public UMassEntityConfigAsset
{
	GENERATED_BODY()
	
protected:
	UBAMassEntityConfigAsset();
	
	virtual void PostInitProperties() override;
	
protected:
	UPROPERTY()
	TObjectPtr<UMassStateTreeTrait> DefaultStateTreeTrait;
};
