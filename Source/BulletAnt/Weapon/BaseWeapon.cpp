#include "BulletAnt/Weapon/BaseWeapon.h"

#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbility.h"
#include "Components/SkeletalMeshComponent.h"
#include "Net/UnrealNetwork.h"


ABaseWeapon::ABaseWeapon()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = false;

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	SetRootComponent(WeaponMesh);

	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ABaseWeapon::EquipWeapon(UAbilitySystemComponent* ASC)
{
	if (!ASC) return;
	
	AActor* Owner = ASC->GetOwner();
	if (!Owner || !Owner->HasAuthority()) return;

	if (GrantedAbilityHandles.Num() > 0) return;

	for (const TSubclassOf<UGameplayAbility>& AbilityClass : GrantedAbilities)
	{
		if (!AbilityClass) continue;

		FGameplayAbilitySpec AbilitySpec(AbilityClass, 1);
		AbilitySpec.SourceObject = this;

		FGameplayAbilitySpecHandle Handle = ASC->GiveAbility(AbilitySpec);
		GrantedAbilityHandles.Add(Handle);
	}
}

void ABaseWeapon::UnequipWeapon(UAbilitySystemComponent* ASC)
{
	if (!ASC) return;
	
	AActor* Owner = ASC->GetOwner();
	if (!Owner || !Owner->HasAuthority()) return;

	for (const FGameplayAbilitySpecHandle& Handle : GrantedAbilityHandles)
	{
		if (Handle.IsValid())
		{
			ASC->ClearAbility(Handle);
		}
	}

	GrantedAbilityHandles.Empty();
}
