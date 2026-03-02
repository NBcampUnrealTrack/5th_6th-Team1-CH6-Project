#include "Weapon/MeleeWeapon/AnimNotifyState_MeleeHitCheck.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Common/DataAssetInterface.h"
#include "Engine/OverlapResult.h"
#include "Weapon/Data/MeleeWeaponDataAsset.h"

UAnimNotifyState_MeleeHitCheck::UAnimNotifyState_MeleeHitCheck()
{
}

void UAnimNotifyState_MeleeHitCheck::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	OwnerActor = MeshComp->GetOwner();

	if (IDataAssetInterface* Interface = Cast<IDataAssetInterface>(OwnerActor))
	{
		Data = Cast<UMeleeWeaponDataAsset>(Interface->GetDataAsset());
		if (!Data) return;
	}

	HitActors.Empty();
}

void UAnimNotifyState_MeleeHitCheck::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);

	if (!MeshComp) return;

	if (!OwnerActor)
	{
		OwnerActor = MeshComp->GetOwner();
		if (!OwnerActor) return;
	}

	if (!Data)
	{
		if (IDataAssetInterface* Interface = Cast<IDataAssetInterface>(OwnerActor))
		{
			Data = Cast<UMeleeWeaponDataAsset>(Interface->GetDataAsset());
		}
	}
	if (!Data) return;

	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OwnerActor);
	if (!ASC) return;

	FVector Origin;

	if (Data->SocketName.IsNone())
	{
		Origin = OwnerActor->GetActorLocation() + OwnerActor->GetActorForwardVector() * 50.f;
	}
	else
	{
		Origin = MeshComp->GetSocketLocation(Data->SocketName);
	}

	TArray<FOverlapResult> Overlaps;
	FCollisionQueryParams Params(NAME_None, false, OwnerActor);
	Params.AddIgnoredActors(HitActors);

	bool bHit = OwnerActor->GetWorld()->OverlapMultiByChannel(
		Overlaps,
		Origin,
		FQuat::Identity,
		ECC_GameTraceChannel2,
		FCollisionShape::MakeSphere(Data->AttackRadius),
		Params
	);

	DrawDebugSphere(
		OwnerActor->GetWorld(),
		Origin,
		Data->AttackRadius,
		12,
		bHit ? FColor::Green : FColor::Red,
		false,
		-1.f                         // 1frame
	);

	if (bHit)
	{
		for (const FOverlapResult& Overlap : Overlaps)
		{
			if (AActor* OverlapActor = Overlap.GetActor())
			{
				if (HitActors.Contains(OverlapActor)) continue;
				if (OverlapActor == OwnerActor) continue;

				HitActors.Add(OverlapActor);

				FGameplayEventData Payload;
				Payload.Target = OverlapActor;
				Payload.TargetData = UAbilitySystemBlueprintLibrary::AbilityTargetDataFromActor(OverlapActor);
				FGameplayTag EventTag = Data->HitEventTag;

				ASC->HandleGameplayEvent(EventTag, &Payload);
			}
		}
	}
}

void UAnimNotifyState_MeleeHitCheck::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);
}
