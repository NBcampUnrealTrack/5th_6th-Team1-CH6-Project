#include "BulletAnt/Weapon/BaseWeapon.h"

#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbility.h"
#include "Components/SkeletalMeshComponent.h"
#include "Net/UnrealNetwork.h"

static const FGameplayTag TAG_Event_Weapon_Switch = FGameplayTag::RequestGameplayTag(TEXT("Event.Weapon.Switch"));

ABaseWeapon::ABaseWeapon()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	SetRootComponent(WeaponMesh);

	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ABaseWeapon::EquipWeapon(UAbilitySystemComponent* ASC)
{
	if (!ASC) return;
	AActor* OwnerActor = ASC->GetOwner();
	if (!OwnerActor || !OwnerActor->HasAuthority()) return;

	FGameplayEventData Payload;
	Payload.EventTag = TAG_Event_Weapon_Switch;
	Payload.OptionalObject = this;
	Payload.Instigator = ASC->GetOwner();

	ASC->HandleGameplayEvent(TAG_Event_Weapon_Switch, &Payload);
}

void ABaseWeapon::UnequipWeapon(UAbilitySystemComponent* ASC)
{
	if (!ASC) return;
	AActor* OwnerActor = ASC->GetOwner();
	if (!OwnerActor || !OwnerActor->HasAuthority()) return;

	for (const FGameplayAbilitySpecHandle& Handle : GrantedAbilityHandles)
	{
		if (Handle.IsValid())
		{
			ASC->ClearAbility(Handle);
		}
	}

	GrantedAbilityHandles.Empty();
}
