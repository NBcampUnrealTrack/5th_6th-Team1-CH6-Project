#include "Weapon/MeleeWeapon/AnimNotifyState_MeleeHitCheck.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Common/DataAssetInterface.h"

#include "Weapon/Data/MeleeWeaponDataAsset.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GAS/BAGameplayTags.h"

UAnimNotifyState_MeleeHitCheck::UAnimNotifyState_MeleeHitCheck()
{
	
}

void UAnimNotifyState_MeleeHitCheck::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (MeshComp && MeshComp->GetOwner())
	{
		FGameplayEventData Payload;
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(MeshComp->GetOwner(), TAG_Event_Combat_StartAttack,Payload);
	}
}

void UAnimNotifyState_MeleeHitCheck::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);

	
}

void UAnimNotifyState_MeleeHitCheck::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	if (MeshComp && MeshComp->GetOwner())
	{
		FGameplayEventData Payload;
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(MeshComp->GetOwner(), TAG_Event_Combat_EndAttack, Payload);
	}
	Super::NotifyEnd(MeshComp, Animation, EventReference);	
}
