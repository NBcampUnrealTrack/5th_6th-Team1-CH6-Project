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

	/*Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);*/

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);
	Mesh->SetSimulatePhysics(false);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Movement"));

	ProjectileMovement->bAutoActivate = false;
	ProjectileMovement->bInterpMovement = true;
	ProjectileMovement->bInterpRotation = true;
	ProjectileMovement->bShouldBounce = false;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bRotationRemainsVertical = true;

	ProjectileMovement->UpdatedComponent = Mesh;
}

void ABAItemBox::Use_Implementation(AActor* User)
{
	ABACharacter* Player = Cast<ABACharacter>(User);
	if (!Player) return;

	ABAPlayerController* PC = Cast<ABAPlayerController>(Player->GetController());
	if (!PC) return;

	if (Item)
	{
		PC->Server_RequestAddWeapon(Item);
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
	}
	ProjectileMovement->Activate();
}


