// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/BaseRangedWeapon.h"

FVector ABaseRangedWeapon::GetFireStartLocation() const
{
	if (WeaponMesh && WeaponMesh->DoesSocketExist(MuzzleSocketName))
	{
		return WeaponMesh->GetSocketLocation(MuzzleSocketName);
	}

	return GetActorLocation();
}

FVector ABaseRangedWeapon::GetFireDirection() const
{
	if (WeaponMesh && WeaponMesh->DoesSocketExist(MuzzleSocketName))
	{
		return WeaponMesh->GetSocketRotation(MuzzleSocketName).Vector();
	}

	return GetActorForwardVector();
}
