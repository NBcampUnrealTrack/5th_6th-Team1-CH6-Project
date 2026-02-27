// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Spawn/TribeMaterialManagerSubsystem.h"

UMaterialInstanceDynamic* UTribeMaterialManagerSubsystem::GetTribeMaterial(UMaterialInterface* InBaseMat, const FLinearColor& InColor)
{
	if (!ensureMsgf(IsValid(InBaseMat), TEXT("TribeMaterialManagerSubsystem GetTribeMaterial : InBaseMaterial Error")))
	{
		return nullptr;
	}

	FTribeMaterialKey key(InBaseMat, InColor);
	if (TribeMaterialCache.Contains(key))
	{
		return TribeMaterialCache[key];
	}

	UMaterialInstanceDynamic* MID = UMaterialInstanceDynamic::Create(InBaseMat, this);
	if (IsValid(MID))
	{
		MID->SetVectorParameterValue(TEXT("AdditiveColor"), InColor);
		TribeMaterialCache.Add(key, MID);
		return MID;
	}

	return nullptr;
}
