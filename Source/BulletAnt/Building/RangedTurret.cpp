// Fill out your copyright notice in the Description page of Project Settings.


#include "Building/RangedTurret.h"
#include "AbilitySystemComponent.h"
#include "Engine/StaticMeshSocket.h"
#include "Weapon/Abilities/GA_Fire.h"
#include "Building/RangedTurretDataAsset.h"
#include "Weapon/Data/RangedWeaponDataAsset.h"

ARangedTurret::ARangedTurret()
{
}

void ARangedTurret::BeginPlay()
{
	TurretData = RangedTurretData;

	Super::BeginPlay();

	CollectMuzzleSockets();

	if (HasAuthority() && ASC)
	{
		ASC->GiveAbility(FGameplayAbilitySpec(UGA_Fire::StaticClass(), 1));
	}
}

UDataAsset* ARangedTurret::GetDataAsset() const
{
	return RangedTurretData ? Cast<UDataAsset>(RangedTurretData->WeaponData) : nullptr;
}

FVector ARangedTurret::GetFireStartLocation_Implementation() const
{
	if (BarrelMesh && MuzzleSocketNames.IsValidIndex(CurrentMuzzleIndex))
	{
		const FName SocketName = MuzzleSocketNames[CurrentMuzzleIndex];
		if (BarrelMesh->DoesSocketExist(SocketName))
		{
			return BarrelMesh->GetSocketLocation(SocketName);
		}
	}

	return GetActorLocation();
}

FVector ARangedTurret::GetFireDirection_Implementation() const
{
	if (BarrelMesh && MuzzleSocketNames.IsValidIndex(CurrentMuzzleIndex))
	{
		const FName SocketName = MuzzleSocketNames[CurrentMuzzleIndex];
		if (BarrelMesh->DoesSocketExist(SocketName))
		{
			const FTransform SocketTM = BarrelMesh->GetSocketTransform(SocketName, RTS_World);
			return SocketTM.GetUnitAxis(EAxis::X);
		}
	}

	return GetActorForwardVector();
}

float ARangedTurret::GetAttackInterval() const
{
	if (!RangedTurretData || !RangedTurretData->WeaponData || RangedTurretData->WeaponData->RoundPerMinute <= 0.f)
	{
		return 0.f;
	}

	return 60.f / RangedTurretData->WeaponData->RoundPerMinute;
}

void ARangedTurret::ExecuteAttack()
{
	if (!HasAuthority() || bDead || !ASC || !RangedTurretData || !RangedTurretData->WeaponData || !CurrentTarget)
	{
		return;
	}

	CurrentMuzzleIndex = NextMuzzleIndex;

	FGameplayTagContainer FireTags;
	FireTags.AddTag(RangedTurretData->WeaponData->WeaponTag);
	ASC->TryActivateAbilitiesByTag(FireTags);
}

void ARangedTurret::CollectMuzzleSockets()
{
	MuzzleSocketNames.Empty();

	if (!RangedTurretData || !BarrelMesh)
	{
		return;
	}

	const UStaticMesh* Mesh = BarrelMesh->GetStaticMesh();
	if (!Mesh)
	{
		return;
	}

	for (const UStaticMeshSocket* Socket : Mesh->Sockets)
	{
		if (!Socket)
		{
			continue;
		}

		const FString NameStr = Socket->SocketName.ToString();
		if (NameStr.StartsWith(RangedTurretData->MuzzleSocketPrefix.ToString()))
		{
			MuzzleSocketNames.Add(Socket->SocketName);
		}
	}

	MuzzleSocketNames.Sort([](const FName& A, const FName& B)
		{
			return A.LexicalLess(B);
		});
}

void ARangedTurret::OnDeath()
{
	Super::OnDeath();

	if (ASC && RangedTurretData && RangedTurretData->WeaponData)
	{
		FGameplayTagContainer FireTags;
		FireTags.AddTag(RangedTurretData->WeaponData->WeaponTag);
		ASC->CancelAbilities(&FireTags, nullptr, nullptr);
	}
}
