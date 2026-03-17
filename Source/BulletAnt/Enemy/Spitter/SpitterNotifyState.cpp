// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Spitter/SpitterNotifyState.h"
#include "Enemy/Spitter/BaseSpitterEnemy.h"

void USpitterNotifyState::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	ABaseSpitterEnemy* Owner = Cast<ABaseSpitterEnemy>(MeshComp->GetOwner());
	if (IsValid(Owner))
	{
		Owner->StartSpit();
	}
}

void USpitterNotifyState::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	ABaseSpitterEnemy* Owner = Cast<ABaseSpitterEnemy>(MeshComp->GetOwner());
	if (IsValid(Owner))
	{
		Owner->StopSpit();
	}

	Super::NotifyEnd(MeshComp, Animation, EventReference);
}
