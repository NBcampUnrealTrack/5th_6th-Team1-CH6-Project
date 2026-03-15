#include "Shop/BAItemBox.h"

#include "Player/BACharacter.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Player/BAPlayerController.h"
#include "Weapon/BaseWeapon.h"

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
	if (bIsUsed) return;

	ABACharacter* Player = Cast<ABACharacter>(User);
	if (!Player) return;

	ABAPlayerController* PC = Cast<ABAPlayerController>(Player->GetController());
	if (!PC) return;

	if (bIsUsed) return;
	
	if (Item)
	{
		PC->Server_RequestAddWeapon(Item);
		PC->Server_RequestDeleteBox(this);
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
	
}

void ABAItemBox::DestroyItemBox()
{
	if (!HasAuthority()) return;
	Destroy();
}


