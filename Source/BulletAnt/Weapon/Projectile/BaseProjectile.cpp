#include "Weapon/Projectile/BaseProjectile.h"

#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GameFramework/Character.h"
#include "AbilitySystemComponent.h"
#include "GAS/BAGameplayTags.h"
#include "AbilitySystemInterface.h"
#include "Weapon/Data/RangedWeaponDataAsset.h"

ABaseProjectile::ABaseProjectile()
{
	PrimaryActorTick.bCanEverTick = false;

	RootComponent = CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("Collision Component"));
	
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisionComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	CollisionComponent->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	CollisionComponent->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
	CollisionComponent->CanCharacterStepUpOn = ECanBeCharacterBase::ECB_No;

	CollisionComponent->OnComponentHit.AddDynamic(this, &ABaseProjectile::OnHit);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Projectile Movement"));

	ProjectileMovement->bShouldBounce = false;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bRotationRemainsVertical = true;

	SetReplicateMovement(true);
	bReplicates = true;
}

void ABaseProjectile::InitProjectile(const FVector& Start, const FVector& Direction, const float Radius, float Speed, float Damage,URangedWeaponDataAsset* Data, AActor* InOwner)
{
	CollisionComponent->SetSphereRadius(Radius);

	ProjectileMovement->InitialSpeed = Speed;
	ProjectileMovement->MaxSpeed = Speed;

	SetActorLocation(Start);
	SetActorRotation(Direction.Rotation());

	CachedDamage = Damage;
	CachedOwner = InOwner;
	CachedData = Data;

	ProjectileMovement->Velocity = Direction * Speed;
}

void ABaseProjectile::ActivateProjectile()
{
	
	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);

	ProjectileMovement->Activate();
}

void ABaseProjectile::DeactivateProjectile()
{
	ProjectileMovement->StopMovementImmediately();
	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);

	//Test
	Destroy();
}

void ABaseProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	DeactivateProjectile();

	if (!HasAuthority()) return;

	if (!OtherActor || OtherActor == GetOwner()) return;

	IAbilitySystemInterface* SourceASCInterface = Cast<IAbilitySystemInterface>(CachedOwner);
	if (!SourceASCInterface) return;
	IAbilitySystemInterface* TargetASCInterface = Cast<IAbilitySystemInterface>(OtherActor);
	if (!TargetASCInterface) return;

	UAbilitySystemComponent* SourceASC = SourceASCInterface->GetAbilitySystemComponent();
	if (!SourceASC) return;

	UAbilitySystemComponent* TargetASC = TargetASCInterface->GetAbilitySystemComponent();
	if (!TargetASC) return;

	if (!IsValid(CachedData)) return;

	FGameplayEffectContextHandle Context = SourceASC->MakeEffectContext();
	Context.AddHitResult(Hit);

	FGameplayEffectSpecHandle Spec =
		SourceASC->MakeOutgoingSpec(CachedData->OnUseStateHitEffect, 1.f, Context);

	if (!Spec.IsValid()) return;

	Spec.Data->SetSetByCallerMagnitude(
		TAG_Data_Combat_Damage,
		CachedDamage
	);

	SourceASC->ApplyGameplayEffectSpecToTarget(*Spec.Data.Get(), TargetASC);
}


