// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Spawn/TribeMaterialManagerSubsystem.h"

UMaterialInstanceDynamic* UTribeMaterialManagerSubsystem::GetTribeMaterial(UMaterialInterface* InBaseMat, const FLinearColor& InColor)
{
	if (!ensureMsgf(IsValid(InBaseMat), TEXT("TribeMaterialManagerSubsystem GetTribeMaterial : InBaseMaterial Error")))
	{
		return nullptr;
	}

	FTribeMaterialKey key(InBaseMat, InColor);
	if (TWeakObjectPtr<UMaterialInstanceDynamic>* FoundPtr = TribeMaterialCache.Find(key))
	{
		if (FoundPtr->IsValid())
		{
			return FoundPtr->Get();
		}
	}

	UMaterialInstanceDynamic* MID = UMaterialInstanceDynamic::Create(InBaseMat, this);
	if (IsValid(MID))
	{
		MID->SetVectorParameterValue(TEXT("AdditiveColor"), InColor);
		
		float Alpha;
		if (InColor == FLinearColor(0, 0, 0, 1))
		{
			Alpha = 0.9f;
		}
		else
		{
			Alpha = 0.3f;
		}
		MID->SetScalarParameterValue(TEXT("Alpha"), Alpha);
		TribeMaterialCache.Add(key, MID);
		return MID;
	}

	return nullptr;
}
