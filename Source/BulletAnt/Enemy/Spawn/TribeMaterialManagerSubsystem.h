// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "TribeMaterialManagerSubsystem.generated.h"

USTRUCT()
struct FTribeMaterialKey
{
    GENERATED_BODY()

    TWeakObjectPtr<UMaterialInterface> BaseMaterial;

    FLinearColor TribeColor = FLinearColor(0, 0, 0, 0);

    // Key Comparison
    bool operator==(const FTribeMaterialKey& Other) const
    {
        return (BaseMaterial == Other.BaseMaterial &&TribeColor == Other.TribeColor);
    }

    // Hash Function
    friend uint32 GetTypeHash(const FTribeMaterialKey& Key)
    {
        uint32 Hash = 0;
        Hash = HashCombine(Hash, GetTypeHash(Key.BaseMaterial));
        Hash = HashCombine(Hash, GetTypeHash(Key.TribeColor));
        return Hash;
    }
};

UCLASS()
class BULLETANT_API UTribeMaterialManagerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
    UMaterialInstanceDynamic* GetTribeMaterial(UMaterialInterface* InBaseMat, const FLinearColor& InColor);

private:
    TMap<FTribeMaterialKey, TWeakObjectPtr<UMaterialInstanceDynamic>> TribeMaterialCache;
};
