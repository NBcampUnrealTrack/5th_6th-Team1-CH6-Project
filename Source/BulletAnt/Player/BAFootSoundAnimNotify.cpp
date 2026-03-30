// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/BAFootSoundAnimNotify.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SkeletalMeshComponent.h"

UBAFootSoundAnimNotify::UBAFootSoundAnimNotify()
{
#if WITH_EDITORONLY_DATA
    NotifyColor = FColor(196, 142, 255, 255);
#endif
    static ConstructorHelpers::FObjectFinder<USoundAttenuation> AttenAsset(TEXT("/Game/Audio/ATT_Footstep.ATT_Footstep"));

    if (AttenAsset.Succeeded())
    {
        FootstepAttenuation = AttenAsset.Object;
    }
}

void UBAFootSoundAnimNotify::Notify(USkeletalMeshComponent * MeshComp, UAnimSequenceBase * Animation, const FAnimNotifyEventReference & EventReference)
{
    Super::Notify(MeshComp, Animation, EventReference);

    if (MeshComp && MetaSoundToPlay)
    {
        UGameplayStatics::PlaySoundAtLocation(
            MeshComp->GetWorld(),
            MetaSoundToPlay,
            MeshComp->GetComponentLocation(),
            VolumeMultiplier,
            1.0f, // PitchMultiplier
            0.0f,  // StartTime
            FootstepAttenuation
        );
    }
}
