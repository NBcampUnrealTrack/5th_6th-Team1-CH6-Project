// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/BACharacterAnimNotify.h"
#include "Player/BACharacter.h"

void UBACharacterAnimNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (MeshComp && MeshComp->GetOwner())
	{
		ABACharacter* Character = Cast<ABACharacter>(MeshComp->GetOwner());
		if (Character)
		{
			Character->StopMontage();
		}
	}
}

