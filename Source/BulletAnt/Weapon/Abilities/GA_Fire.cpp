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
	FireDelay = 60.f / RangedData->RoundPerMinute;
	
	if (RangedData->bAutoFire)
	{
		StartAutoFireLoop();
	}
	else 
	{
		FireOnce();
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
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

void UGA_Fire::FireOnce()
{	
	FVector Start = IFireStartInterface::Execute_GetFireStartLocation(CurrentActorInfo->AvatarActor.Get());
	FVector Dir = IFireStartInterface::Execute_GetFireDirection(CurrentActorInfo->AvatarActor.Get());

	//총알 발사시 발생하는 이펙트 큐

	FGameplayEffectContextHandle Context = CachedASC->MakeEffectContext();
	Context.AddSourceObject(RangedData);
	Context.AddOrigin(Start);

	CachedASC->ExecuteGameplayCue(TAG_GameplayCue_Weapon_Fire, Context);

	if (!CurrentActorInfo || !CurrentActorInfo->IsNetAuthority()) return;

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
	

	GetWorld()->GetTimerManager().SetTimer(
		FireTimerHandler,
		this,
		&UGA_Fire::FireOnce,
		FireDelay,
		true,
		0.f
	);
}


FVector UGA_Fire::ApplySpread(const FVector& Dir, float Degree)
{
	if (Degree <= 0.f) return Dir;

	const float HalfRad = FMath::DegreesToRadians(Degree * 0.5f);
	return FMath::VRandCone(Dir, HalfRad);
}


