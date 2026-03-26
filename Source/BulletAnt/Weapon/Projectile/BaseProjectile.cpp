#include "Weapon/Projectile/BaseProjectile.h"

#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GameFramework/Character.h"
#include "AbilitySystemComponent.h"
#include "GAS/BAGameplayTags.h"
#include "AbilitySystemInterface.h"
#include "Weapon/Data/RangedWeaponDataAsset.h"
#include "NiagaraComponent.h"
#include "GAS/AttributeSet/HealthAttributeSet.h"

ABaseProjectile::ABaseProjectile()
{
	PrimaryActorTick.bCanEverTick = false;

	RootComponent = CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("Collision Component"));
	
	CollisionComponent->SetCollisionProfileName("Bullet");
	CollisionComponent->CanCharacterStepUpOn = ECanBeCharacterBase::ECB_No;

	CollisionComponent->OnComponentHit.AddDynamic(this, &ABaseProjectile::OnHit);

	BulletMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Bullet Mesh"));
	BulletMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	BulletMesh->SetCollisionObjectType(ECC_GameTraceChannel8);

	BulletMesh->SetupAttachment(CollisionComponent);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Projectile Movement"));

	ProjectileMovement->bShouldBounce = false;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bRotationRemainsVertical = true;
	ProjectileMovement->bInterpMovement = true;
	ProjectileMovement->bAutoActivate = false;

	Tracer = CreateDefaultSubobject<UNiagaraComponent>(TEXT("Tracer"));
	Tracer->SetupAttachment(RootComponent);

	SetReplicateMovement(true);
	bReplicates = true;

	
}

void ABaseProjectile::InitProjectile(const FVector& Start, const FVector& Direction, const float Radius, float Speed, float Damage, URangedWeaponDataAsset* Data, AActor* InOwner)
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
	bIsActive = true;
}


void ABaseProjectile::DeactivateProjectile()
{
	ProjectileMovement->StopMovementImmediately();
	ProjectileMovement->Deactivate();
	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);

	bIsActive = false;
	Tracer->Deactivate();
}

void ABaseProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	DeactivateProjectile();

	if (!HasAuthority()) return;

	if (!OtherActor || OtherActor == CachedOwner) return;

	IAbilitySystemInterface* SourceASCInterface = Cast<IAbilitySystemInterface>(CachedOwner);
	if (!SourceASCInterface) return;
	IAbilitySystemInterface* TargetASCInterface = Cast<IAbilitySystemInterface>(OtherActor);
	if (!TargetASCInterface) return;

	UAbilitySystemComponent* SourceASC = SourceASCInterface->GetAbilitySystemComponent();
	if (!SourceASC) return;

	UAbilitySystemComponent* TargetASC = TargetASCInterface->GetAbilitySystemComponent();
	if (!TargetASC) return;

	if (!IsValid(CachedData)) return;

	const UHealthAttributeSet* SourceHealthSet = SourceASC->GetSet<UHealthAttributeSet>();
	float SourceAttackPower = SourceHealthSet ? SourceHealthSet->GetAttackPower() : 0.f;

	float RandomVariance = FMath::FRandRange(0.9f, 1.1f);

	float FinalDamage = (CachedDamage + SourceAttackPower) * RandomVariance;

	FGameplayEffectContextHandle Context = SourceASC->MakeEffectContext();
	Context.AddHitResult(Hit);
	Context.AddInstigator(CachedOwner,this);

	FGameplayEffectSpecHandle Spec =
		SourceASC->MakeOutgoingSpec(CachedData->OnUseStateHitEffect, 1.f, Context);
	
	if (!Spec.IsValid()) return;

	Spec.Data->SetSetByCallerMagnitude(
		TAG_Data_Combat_Damage,
		FinalDamage
	);

	SourceASC->ApplyGameplayEffectSpecToTarget(*Spec.Data.Get(), TargetASC);
}


