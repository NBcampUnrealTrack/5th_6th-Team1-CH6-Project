#include "GA_Fire.h"

#include "Weapon/BaseRangedWeapon.h"
#include "AbilitySystemComponent.h"
#include "DrawDebugHelpers.h"
#include "Weapon/Data/RangedWeaponDataAsset.h"
#include "Abilities/GameplayAbility.h"
#include "Common/DataAssetInterface.h"
#include "Common/FireStartInterface.h" 
#include "Abilities/Tasks/AbilityTask_WaitGameplayTag.h"
#include "GAS/AttributeSet/AmmoAttributeSet.h"
#include "GAS/BAGameplayTags.h"
#include "Weapon/Projectile/BaseProjectile.h"
#include "TimerManager.h"

UGA_Fire::UGA_Fire()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;

	FGameplayTagContainer DefaultTag;
	DefaultTag.AddTag(TAG_Ability_Active_Fire);
	SetAssetTags(DefaultTag);
	
	ActivationOwnedTags.AddTag(TAG_State_Combat_Attacking);
}

void UGA_Fire::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	SourceActor = Cast<AActor>(ActorInfo->AvatarActor.Get());
	if (!SourceActor)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	IDataAssetInterface* DataAssetInterface = Cast<IDataAssetInterface>(SourceActor);
	if (!DataAssetInterface)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	RangedData = Cast<URangedWeaponDataAsset>(DataAssetInterface->GetDataAsset());
	if (!RangedData)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	CachedASC = ActorInfo->AbilitySystemComponent.Get();
	if (!CachedASC) 
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	ContinuousBullet = 0;
	
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

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_Fire::FireOnce(const FGameplayAbilityActorInfo* ActorInfo)
{
	if (!ActorInfo || !ActorInfo->IsNetAuthority()) return;

	const UAmmoAttributeSet* AmmoSet = CachedASC->GetSet<UAmmoAttributeSet>();
	if (AmmoSet)
	{
		if (AmmoSet->GetCurrentAmmo() <= 0.f)
		{
			EndAbility(CurrentSpecHandle, ActorInfo, CurrentActivationInfo, true, false);
			return;
		}
	}

	FVector Start = IFireStartInterface::Execute_GetFireStartLocation(ActorInfo->AvatarActor.Get());
	FVector Dir = IFireStartInterface::Execute_GetFireDirection(ActorInfo->AvatarActor.Get());
	
	//총알 발사시 발생하는 이펙트 큐
	if (RangedData->FireCueEffect)
	{
		FGameplayEffectContextHandle Context = CachedASC->MakeEffectContext();
		Context.AddSourceObject(RangedData);
		Context.AddOrigin(Start);
		

		FGameplayEffectSpecHandle SpecHandle = CachedASC->MakeOutgoingSpec(RangedData->FireCueEffect, 1.0f, Context);
		if (SpecHandle.IsValid())
		{
			CachedASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
		}
	}

	ContinuousBullet++;

	FActorSpawnParameters Params;
	Params.Owner = SourceActor;

	for (int32 i = 0; i < RangedData->FirePerShot; ++i)
	{
		FVector FireDir = ApplySpread(
			Dir,
			RangedData->SpreadDegree
		);

		ABaseProjectile* Prj = GetWorld()->SpawnActor<ABaseProjectile>(
			RangedData->ProjectileClass,
			Start,
			FireDir.Rotation(),
			Params
		);

		Prj->InitProjectile(
			Start,
			FireDir,
			RangedData->ProjectileRadius,
			RangedData->ProjectileSpeed,
			RangedData->BaseDamage,
			RangedData,
			SourceActor
		);
		Prj->ActivateProjectile();
	}
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


FVector UGA_Fire::ApplySpread(const FVector& Dir, float Degree)
{
	if (Degree <= 0.f) return Dir;

	const float HalfRad = FMath::DegreesToRadians(Degree * 0.5f);
	return FMath::VRandCone(Dir, HalfRad);
}


