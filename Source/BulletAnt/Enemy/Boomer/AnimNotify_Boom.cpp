// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Boomer/AnimNotify_Boom.h"
#include "Enemy/BaseEnemy/BaseEnemyCharacter.h"

void UAnimNotify_Boom::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (MeshComp->GetNetMode() != ENetMode::NM_ListenServer)
	{
		return;
	}
	if (!IsValid(MeshComp))
	{
		return;
	}
	AActor* Actor = MeshComp->GetOwner();
	if (!IsValid(Actor))
	{
		return;
	}
	ABaseEnemyCharacter* BaseEnemyCharacter = Cast<ABaseEnemyCharacter>(Actor);
	if (!IsValid(BaseEnemyCharacter))
	{
		return;
	}

	BaseEnemyCharacter->Multicast_SetNoCollision();
}
