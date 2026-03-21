#include "GAS/Ability/GA_MeleeAttack.h"
#include "Common/DataAssetInterface.h"
#include "Weapon/Data/MeleeWeaponDataAsset.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitTargetData.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GAS/BAGameplayTags.h"
#include "GameFramework/Character.h"
#include "Engine/OverlapResult.h"

UGA_MeleeAttack::UGA_MeleeAttack()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

	FGameplayTagContainer DefaultTag;
	DefaultTag.AddTag(TAG_Ability_Active_MeleeAttack);

	SetAssetTags(DefaultTag);

	BlockAbilitiesWithTag.AddTag(TAG_Ability_Active);

	ActivationOwnedTags.AddTag(TAG_State_Combat_Attacking);
}

void UGA_MeleeAttack::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	IDataAssetInterface* Interface = Cast<IDataAssetInterface>(GetAvatarActorFromActorInfo());
	if (!Interface || !Interface->GetDataAsset())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	Data = Cast<UMeleeWeaponDataAsset>(Interface->GetDataAsset());
	if (!Data)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	OwnerActor = Cast<ACharacter>(CurrentActorInfo->AvatarActor.Get());
	if (!OwnerActor)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	MeshComp = OwnerActor->GetMesh();
	if (!MeshComp)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	CachedASC = ActorInfo->AbilitySystemComponent.Get();
	CachedActivationInfo = ActivationInfo;
	CachedHandle = Handle;

	if (IsLocallyControlled())
	{
		UAbilityTask_WaitGameplayEvent* WaitStartTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, TAG_Event_Combat_StartAttack);
		WaitStartTask->EventReceived.AddDynamic(this, &UGA_MeleeAttack::OnStartEventReceived);
		WaitStartTask->ReadyForActivation();

		UAbilityTask_WaitGameplayEvent* WaitEndTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, TAG_Event_Combat_EndAttack);
		WaitEndTask->EventReceived.AddDynamic(this, &UGA_MeleeAttack::OnEndEventReceived);
		WaitEndTask->ReadyForActivation();	
	}

	if (ActorInfo->IsNetAuthority())
	{
		UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();

		ASC->AbilityTargetDataSetDelegate(
			GetCurrentAbilitySpecHandle(),
			GetCurrentActivationInfo().GetActivationPredictionKey()
		).AddUObject(this, &UGA_MeleeAttack::OnTargetDataReadyCallback);
	}

	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, Data->AttackMontage);
	MontageTask->OnCompleted.AddDynamic(this, &UGA_MeleeAttack::OnMontageFinished);
	MontageTask->OnInterrupted.AddDynamic(this, &UGA_MeleeAttack::OnMontageFinished);
	MontageTask->OnCancelled.AddDynamic(this, &UGA_MeleeAttack::OnMontageFinished);
	MontageTask->ReadyForActivation();
}

void UGA_MeleeAttack::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_MeleeAttack::OnStartEventReceived(FGameplayEventData Payload)
{
	if (Data->SocketName.IsNone())
	{
		StartLocation = OwnerActor->GetActorLocation() + OwnerActor->GetActorForwardVector() * 50.f;
	}
	else
	{
		StartLocation = MeshComp->GetSocketLocation(Data->SocketName);
	}

	HitActors.Empty();
	GetWorld()->GetTimerManager().SetTimer(
		HitCheckTimer,
		this,
		&UGA_MeleeAttack::PerformHitCheck,
		0.1f,
		true
	);
}

void UGA_MeleeAttack::OnEndEventReceived(FGameplayEventData Payload)
{
	GetWorld()->GetTimerManager().ClearTimer(HitCheckTimer);
}

void UGA_MeleeAttack::OnMontageFinished()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_MeleeAttack::PerformHitCheck()
{
	if (Data->SocketName.IsNone())
	{
		EndLocation = OwnerActor->GetActorLocation() + OwnerActor->GetActorForwardVector() * 50.f;
	}
	else
	{
		EndLocation = MeshComp->GetSocketLocation(Data->SocketName);
	}

	TArray<FHitResult> Hits;
	FCollisionQueryParams Params(NAME_None, false, OwnerActor);
	Params.AddIgnoredActors(HitActors);

	bool bHit = OwnerActor->GetWorld()->SweepMultiByChannel(
		Hits,
		StartLocation,
		EndLocation,
		FQuat::Identity,
		ECC_GameTraceChannel2,
		FCollisionShape::MakeSphere(Data->AttackRadius),
		Params
	);
	FGameplayAbilityTargetDataHandle TargetData;
	
	if (bHit)
	{
		for (const FHitResult& Hit : Hits)
		{
			if (AActor* HitActor = Hit.GetActor())
			{
				if (HitActors.Contains(HitActor)) continue;
				if (HitActor == OwnerActor) continue;

				HitActors.Add(HitActor);

				FGameplayAbilityTargetData_SingleTargetHit* HitData =
					new FGameplayAbilityTargetData_SingleTargetHit(Hit);

				TargetData.Add(HitData);
				
			}
		}
		if (TargetData.Num() > 0)
		{
			GetAbilitySystemComponentFromActorInfo()->ServerSetReplicatedTargetData(
				CachedHandle,
				CachedActivationInfo.GetActivationPredictionKey(),
				TargetData,
				FGameplayTag(),
				CachedASC->ScopedPredictionKey
			);
		}
	}
	StartLocation = EndLocation;
}

void UGA_MeleeAttack::OnTargetDataReadyCallback(
	const FGameplayAbilityTargetDataHandle& InData,
	FGameplayTag ActivationTag)
{
	if (!CurrentActorInfo->IsNetAuthority()) return;

	for (int32 i = 0; i < InData.Num(); ++i)
	{
		FGameplayAbilityTargetData* TargetData =
			const_cast<FGameplayAbilityTargetData*>(InData.Get(i));

		if (TargetData->GetScriptStruct() == FGameplayAbilityTargetData_SingleTargetHit::StaticStruct())
		{
			auto* HitData = (FGameplayAbilityTargetData_SingleTargetHit*)TargetData;

			ApplyDamage(CurrentActorInfo, HitData->HitResult);
		}
	}
}

void UGA_MeleeAttack::ApplyDamage(const FGameplayAbilityActorInfo* ActorInfo, FHitResult& InHit)
{
	if (!ActorInfo) return;

	AActor* Target = InHit.GetActor();

	UAbilitySystemComponent* SourceASC = ActorInfo->AbilitySystemComponent.Get();
	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Target);

	if (SourceASC && TargetASC)
	{
		if (!Data) return;

		if (Data->OnUseStateHitEffect)
		{
			FGameplayEffectContextHandle Context = SourceASC->MakeEffectContext();
			Context.AddInstigator(ActorInfo->OwnerActor.Get(), ActorInfo->AvatarActor.Get());
			Context.AddHitResult(InHit);

			FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(Data->OnUseStateHitEffect, GetAbilityLevel(), Context);

			if (SpecHandle.IsValid())
			{
				SpecHandle.Data->SetSetByCallerMagnitude(TAG_Data_Combat_Damage, Data->BaseDamage);
				SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
			}
		}
	}
}
