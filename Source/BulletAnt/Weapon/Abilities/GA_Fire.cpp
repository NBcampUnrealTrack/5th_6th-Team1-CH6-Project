#include "GA_Fire.h"

#include "Weapon/BaseRangedWeapon.h"
#include "AbilitySystemComponent.h"
#include "DrawDebugHelpers.h"
#include "Weapon/Data/RangedWeaponDataAsset.h"
#include "Abilities/GameplayAbility.h"
#include "Common/DataAssetInterface.h"
#include "Common/FireStartInterface.h" 
#include "Abilities/Tasks/AbilityTask_WaitGameplayTag.h"

UGA_Fire::UGA_Fire()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

	AbilityTags.AddTag(FGameplayTag::RequestGameplayTag(TEXT("Ability.Weapon.Fire")));

	TAG_Data_Combat_Damage = FGameplayTag::RequestGameplayTag(TEXT("Data.Combat.Damage"));

	TAG_GameplayCue_Weapon_Fire = FGameplayTag::RequestGameplayTag(TEXT("GameplayCue.Weapon.Fire"));
}

void UGA_Fire::ActivateAbility(
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
	SourceActor = Cast<AActor>(ActorInfo->AvatarActor);
	if (!SourceActor) return;

	IDataAssetInterface* DataAssetInterface = Cast<IDataAssetInterface>(SourceActor);
	if (!DataAssetInterface) return;

	RangedData = Cast<URangedWeaponDataAsset>(DataAssetInterface->GetDataAsset());
	if (!RangedData) return;

	
	if (RangedData->bAutoFire)
	{
		UAbilityTask_WaitGameplayTagRemoved* WaitTagRemoved = UAbilityTask_WaitGameplayTagRemoved::WaitGameplayTagRemove(
			this,
			FGameplayTag::RequestGameplayTag(TEXT("State.Input.Attacking"))
		);
		WaitTagRemoved->Removed.AddDynamic(this, &UGA_Fire::OnFireReleased);
		WaitTagRemoved->ReadyForActivation();

		StartAutoFireLoop();
	}
	else 
	{
		FireOnce(ActorInfo);
		EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
	}	
}

void UGA_Fire::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(FireTimerHandler);
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_Fire::FireOnce(const FGameplayAbilityActorInfo* ActorInfo)
{
	if (!ActorInfo) return;

	IFireStartInterface* FireStart = Cast<IFireStartInterface>(SourceActor);
	if (!FireStart) return;
	
	const FVector Start = FireStart->GetFireStartLocation();
	

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(SourceActor);

	for (int32 i = 0; i < RangedData->FirePerShot; ++i)
	{
		FHitResult LocalHit;

		FVector FireDir = ApplySpread(
			FireStart->GetFireDirection(),
			RangedData->SpreadDegree
		);

		const FVector End = Start + FireDir * RangedData->Range;

		bool bHit = SourceActor->GetWorld()->LineTraceSingleByChannel(
			LocalHit,
			Start,
			End,
			ECC_Visibility,
			Params
		);

		DrawDebugLine(
			SourceActor->GetWorld(),
			Start,
			bHit ? LocalHit.ImpactPoint : End,
			FColor::Red,
			false,
			1.f,
			0,
			1.f
		);

		if (!bHit)
		{
			continue;
		}

		ApplyDamageEffect(
			ActorInfo,
			LocalHit.GetActor(),
			RangedData->BaseDamage,
			RangedData->HitEventTag
		);
	}

	ApplyAttackCue(Start, RangedData, ActorInfo->AbilitySystemComponent.Get());
}

void UGA_Fire::StartAutoFireLoop()
{
	const FGameplayAbilityActorInfo* ActorInfo = GetCurrentActorInfo();

	FireOnce(ActorInfo);

	float FireDelay = 60.f / RangedData->RoundPerMinute;

	GetWorld()->GetTimerManager().SetTimer(
		FireTimerHandler,
		this,
		&UGA_Fire::StartAutoFireLoop,
		FireDelay,
		false
	);
}

void UGA_Fire::ApplyDamageEffect(
	const FGameplayAbilityActorInfo* ActorInfo,
	AActor* Target,
	float Damage,
	FGameplayTag HitTag)
{
	if (!Target) return;

	UAbilitySystemComponent* SourceASC = ActorInfo->AbilitySystemComponent.Get();
	if (!SourceASC) return;

	UAbilitySystemComponent* TargetASC =
		Target->FindComponentByClass<UAbilitySystemComponent>();
	if (!TargetASC) return;

	FGameplayEffectSpecHandle Spec =
		SourceASC->MakeOutgoingSpec(DamageEffectClass, 1.f, SourceASC->MakeEffectContext());

	if (!Spec.IsValid()) return;

	Spec.Data->SetSetByCallerMagnitude(
		TAG_Data_Combat_Damage,
		Damage
	);

	SourceASC->ApplyGameplayEffectSpecToTarget(*Spec.Data.Get(), TargetASC);
}

void UGA_Fire::ApplyAttackCue(const FVector& Location, const URangedWeaponDataAsset* WeaponData, UAbilitySystemComponent* ASC)
{
	if (!WeaponData || !ASC) return;
	FGameplayCueParameters Params;
	Params.Location = Location;
	Params.SourceObject = WeaponData;

	ASC->ExecuteGameplayCue(
		TAG_GameplayCue_Weapon_Fire,
		Params
	);
}

FVector UGA_Fire::ApplySpread(const FVector& Dir, float Degree)
{
	if (Degree <= 0.f) return Dir;

	const float HalfRad = FMath::DegreesToRadians(Degree * 0.5f);
	return FMath::VRandCone(Dir, HalfRad);
}

void UGA_Fire::OnFireReleased()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

