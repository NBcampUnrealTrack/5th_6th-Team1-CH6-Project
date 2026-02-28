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

	AbilityTags.AddTag(FGameplayTag::RequestGameplayTag(TEXT("Ability.Active.Fire")));

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
	if (!SourceActor) { EndAbility(Handle, ActorInfo, ActivationInfo, true, true); return; }

	IDataAssetInterface* DataAssetInterface = Cast<IDataAssetInterface>(SourceActor);
	if (!DataAssetInterface) { EndAbility(Handle, ActorInfo, ActivationInfo, true, true); return; }

	RangedData = Cast<URangedWeaponDataAsset>(DataAssetInterface->GetDataAsset());
	if (!RangedData) { EndAbility(Handle, ActorInfo, ActivationInfo, true, true); return; }

	if (!RangedData->UseStateEffect) { EndAbility(Handle, ActorInfo, ActivationInfo, true, true); return; }
	const UGameplayEffect* EffectCDO = RangedData->UseStateEffect->GetDefaultObject<UGameplayEffect>();

	AttackingStateHandle = ApplyGameplayEffectToOwner(Handle, ActorInfo, ActivationInfo, EffectCDO, 1.f, 1);
	
	if (RangedData->bAutoFire)
	{
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

	if (AttackingStateHandle.IsValid())
	{
		if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
		{
			ASC->RemoveActiveGameplayEffect(AttackingStateHandle);
		}
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_Fire::FireOnce(const FGameplayAbilityActorInfo* ActorInfo)
{
	if (!ActorInfo || !ActorInfo->IsNetAuthority()) return;

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
			RangedData
		);
	}

	ApplyAttackCue(Start, RangedData, ActorInfo->AbilitySystemComponent.Get());
}

void UGA_Fire::StartAutoFireLoop()
{
	const FGameplayAbilityActorInfo* ActorInfo = GetCurrentActorInfo();

	FireOnce(ActorInfo);

	float RPM = FMath::Max(RangedData->RoundPerMinute, 1.f);
	float FireDelay = 60.f / RPM;

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
	const URangedWeaponDataAsset* WeaponData)
{
	if (!Target) return;

	UAbilitySystemComponent* SourceASC = ActorInfo->AbilitySystemComponent.Get();
	if (!SourceASC) return;

	UAbilitySystemComponent* TargetASC =
		Target->FindComponentByClass<UAbilitySystemComponent>();
	if (!TargetASC) return;

	FGameplayEffectSpecHandle Spec =
		SourceASC->MakeOutgoingSpec(WeaponData->OnUseStateHitEffect, 1.f, SourceASC->MakeEffectContext());

	if (!Spec.IsValid()) return;

	Spec.Data->SetSetByCallerMagnitude(
		TAG_Data_Combat_Damage,
		RangedData->BaseDamage
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

