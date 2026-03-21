#include "Weapon/Mining/MiningNotify.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "GAS/BAGameplayTags.h"

void UMiningNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!MeshComp) return;

	AActor* Owner = MeshComp->GetOwner();
	if (!Owner) return;

	FGameplayEventData Payload;

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
		Owner,
		TAG_Event_Mining_Hit,
		Payload
	);
}
