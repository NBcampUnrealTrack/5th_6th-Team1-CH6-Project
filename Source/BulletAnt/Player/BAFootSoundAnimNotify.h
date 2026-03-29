// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "BAFootSoundAnimNotify.generated.h"

/**
 * 
 */
UCLASS()
class BULLETANT_API UBAFootSoundAnimNotify : public UAnimNotify
{
	GENERATED_BODY()
	
public:
    UBAFootSoundAnimNotify();

    virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

    UPROPERTY(EditAnywhere, Category = "Audio")
    USoundBase* MetaSoundToPlay;

    UPROPERTY()
    class USoundAttenuation* FootstepAttenuation;

    UPROPERTY(EditAnywhere, Category = "Audio")
    float VolumeMultiplier = 1.0f;
};
