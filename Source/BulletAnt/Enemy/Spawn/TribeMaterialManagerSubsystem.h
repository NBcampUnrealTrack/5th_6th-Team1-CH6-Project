// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "TribeMaterialManagerSubsystem.generated.h"

USTRUCT()
struct FTribeMaterialKey
{
    GENERATED_BODY()

    UPROPERTY()
    TObjectPtr<UMaterialInterface> BaseMaterial;

    UPROPERTY()
    FLinearColor TribeColor = FLinearColor(0, 0, 0, 0);

    // Key Comparison
    bool operator==(const FTribeMaterialKey& Other) const
    {
        return (TribeColor == Other.TribeColor && BaseMaterial == Other.BaseMaterial);
    }

    // Hash Function
    friend uint32 GetTypeHash(const FTribeMaterialKey& Key)
    {
        return HashCombine(GetTypeHash(Key.TribeColor), GetTypeHash(Key.BaseMaterial));
    }
};

UCLASS()
class BULLETANT_API UTribeMaterialManagerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
    UMaterialInstanceDynamic* GetTribeMaterial(UMaterialInterface* InBaseMat, const FLinearColor& InColor);

private:
    TMap<FTribeMaterialKey, TObjectPtr<UMaterialInstanceDynamic>> TribeMaterialCache;
};
