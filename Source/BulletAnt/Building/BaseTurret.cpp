// Fill out your copyright notice in the Description page of Project Settings.


#include "Building/BaseTurret.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagContainer.h"
#include "Net/UnrealNetwork.h"
#include "Weapon/Abilities/GA_Fire.h"
#include "Weapon/Data/RangedWeaponDataAsset.h"

ABaseTurret::ABaseTurret()
{
	bReplicates = true;

	ASC = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("ASC"));
	ASC->SetIsReplicated(true);
	ASC->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);
}

void ABaseTurret::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		ASC->InitAbilityActorInfo(this, this);
		GiveDefaultAbilities();

		StartAutoFire();
	}
}

UAbilitySystemComponent* ABaseTurret::GetAbilitySystemComponent() const
{
	return ASC;
}

FVector ABaseTurret::GetFireStartLocation() const
{
	if (StaticMeshComp && StaticMeshComp->DoesSocketExist(MuzzleSocketName))
	{
		return StaticMeshComp->GetSocketLocation(MuzzleSocketName);
	}

	return GetActorLocation();
}

FVector ABaseTurret::GetFireDirection() const
{
	if (StaticMeshComp && StaticMeshComp->DoesSocketExist(MuzzleSocketName))
	{
		const FTransform SocketTM = StaticMeshComp->GetSocketTransform(MuzzleSocketName, RTS_World);
		return SocketTM.GetUnitAxis(EAxis::X);
	}

	return GetActorForwardVector();
}

UDataAsset* ABaseTurret::GetDataAsset() const
{
	return TurretData;
}

void ABaseTurret::GiveDefaultAbilities()
{
	if (HasAuthority())
	{
		ASC->GiveAbility(FGameplayAbilitySpec(UGA_Fire::StaticClass(), 1));
	}
}

void ABaseTurret::StartAutoFire()
{
	if (HasAuthority() && TurretData)
	{
		const float AttackRate = TurretData->AttackRate;
		GetWorldTimerManager().SetTimer(
			FireTimerHandle,
			this,
			&ABaseTurret::Server_FireTick,
			AttackRate,
			true
		);
	}
	
}

void ABaseTurret::Server_FireTick()
{
	if (HasAuthority())
	{
		const FGameplayTag FireAbilityTag = TurretData->WeaponTag;

		FGameplayTagContainer AbilityTags;
		AbilityTags.AddTag(FireAbilityTag);

		ASC->TryActivateAbilitiesByTag(AbilityTags);
	}
	
}
