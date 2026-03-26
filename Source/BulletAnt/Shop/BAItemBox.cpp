#include "Shop/BAItemBox.h"

#include "Player/BACharacter.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Player/BAPlayerController.h"
#include "Weapon/BaseWeapon.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"

ABAItemBox::ABAItemBox()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicateMovement(true);

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);
	Mesh->SetSimulatePhysics(false);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Movement"));

	ProjectileMovement->bAutoActivate = false;
	ProjectileMovement->bInterpMovement = false;
	ProjectileMovement->bInterpRotation = true;
	ProjectileMovement->bShouldBounce = true;
	ProjectileMovement->Bounciness = 0.03f;
	ProjectileMovement->Friction = 0.6f;
	ProjectileMovement->bBounceAngleAffectsFriction = true;
	ProjectileMovement->bRotationFollowsVelocity = false;
	ProjectileMovement->bRotationRemainsVertical = false;
	ProjectileMovement->bInitialVelocityInLocalSpace = false;

	ProjectileMovement->UpdatedComponent = Mesh;
}

void ABAItemBox::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, bIsUsed);
}

void ABAItemBox::Use_Implementation(AActor* User)
{
	if (!IsValid(this)) return;

	if (bIsUsed) return;

	ABACharacter* Player = Cast<ABACharacter>(User);
	if (!Player) return;

	ABAPlayerController* PC = Cast<ABAPlayerController>(Player->GetController());
	if (!PC) return;

	PC->Server_RequestAddWeapon(this);
}

void ABAItemBox::GetInteractionOptions_Implementation(AActor* User, TArray<FInteractionOption>& OutOptions) const
{
	FInteractionOption Op1;
	Op1.Key = EKeys::F;
	Op1.Label = FText::FromString(TEXT("줍기"));
	Op1.ActionName = TEXT("Pick");
	OutOptions.Add(Op1);
}

void ABAItemBox::Interaction_Implementation(AActor* User, FName ActionName)
{
	if (ActionName == TEXT("Pick"))
	{
		Use_Implementation(User);
	}
}

void ABAItemBox::SetItem(TSubclassOf<ABaseWeapon> InItem)
{
	Item = InItem;
}

void ABAItemBox::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		ProjectileMovement->ProjectileGravityScale = 1.f;

		ProjectileMovement->InitialSpeed = 0.f;
		ProjectileMovement->MaxSpeed = 0.f;
		ProjectileMovement->Velocity = FVector::ZeroVector;

		ProjectileMovement->Activate();
	}
	ProjectileMovement->OnProjectileStop.AddDynamic(this, &ABAItemBox::Multi_PlayDropSound);
}

void ABAItemBox::Multi_PlayDropSound_Implementation(const FHitResult& ImpactPoint)
{
	if (DropSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			GetWorld(),
			DropSound,
			GetActorLocation()
		);
	}

	if (DropEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			DropEffect,
			ImpactPoint.ImpactPoint,
			ImpactPoint.ImpactNormal.Rotation()
		);
	}
}

void ABAItemBox::DestroyItemBox()
{
	if (!HasAuthority()) return;
	SetLifeSpan(0.1f);
}


