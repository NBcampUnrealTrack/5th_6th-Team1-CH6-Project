#include "BulletAnt/Weapon/BaseWeapon.h"

#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbility.h"
#include "Components/SkeletalMeshComponent.h"
#include "Net/UnrealNetwork.h"
#include "GAS/BAGameplayTags.h"

ABaseWeapon::ABaseWeapon()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	RootComp = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(RootComp);

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	WeaponMesh->SetupAttachment(RootComp);

	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	WeaponMesh->SetRenderCustomDepth(true);
	WeaponMesh->CustomDepthStencilValue = 1;
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

void ABaseWeapon::SetStencilValue(int32 NewValue)
{
	WeaponMesh->SetCustomDepthStencilValue(NewValue);
}
