#include "Weapon/BaseRangedWeapon.h"
#include "Components/SkeletalMeshComponent.h"

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

