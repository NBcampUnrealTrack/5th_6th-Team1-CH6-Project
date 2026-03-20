#include "Shop/BATransportShip.h"

#include "Components/SplineComponent.h"
#include "Shop/BAItemBox.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Weapon/BaseWeapon.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"

ABATransportShip::ABATransportShip()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	SetReplicateMovement(true);

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	PlaneMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlaneMesh"));
	PlaneMesh->SetupAttachment(Root);
	PlaneMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Projectile Movement"));

	ProjectileMovement->bShouldBounce = false;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bRotationRemainsVertical = true;
	ProjectileMovement->bInterpMovement = true;
	ProjectileMovement->bAutoActivate = false;
	ProjectileMovement->ProjectileGravityScale = 0.f;
	ProjectileMovement->SetInterpolatedComponent(Root);
}

void ABATransportShip::InitItemPlane(FVector& InDropLocation, TSubclassOf<ABaseWeapon> InItem)
{
	if (!HasAuthority()) return;
	if (!InItem) return;

	bIsPlayer = false;
	Item = InItem;

	float DirectionX = FMath::RandRange(-1.f, 1.f);
	float DirectionY = FMath::RandRange(-1.f, 1.f);
	FVector Direction = FVector(DirectionX, DirectionY, 0.f).GetSafeNormal();
	
	FVector DropLocation = InDropLocation;
	DropLocation.X += FMath::RandRange(-1000.f, 1000.f);
	DropLocation.Y += FMath::RandRange(-1000.f, 1000.f);

	FVector Start = DropLocation - Direction * TotalDistance / 2;
	FVector End = DropLocation + Direction * TotalDistance / 2;

	SetActorRotation(Direction.Rotation());
	SetActorLocation(Start);

	ProjectileMovement->InitialSpeed = Speed;
	ProjectileMovement->MaxSpeed = Speed;
	ProjectileMovement->Velocity = Direction * Speed;

	bIsDropped = false;
	CurrentDistance = 0.f;
	ProjectileMovement->Activate();
}

void ABATransportShip::InitPlayerPlane(FVector& InDropLocation, ACharacter* PlayerCharacter)
{
	if (!HasAuthority()) return;

	bIsPlayer = true;
	CachedPlayerCharacter = PlayerCharacter;

	float DirectionX = FMath::RandRange(-1.f, 1.f);
	float DirectionY = FMath::RandRange(-1.f, 1.f);
	FVector Direction = FVector(DirectionX, DirectionY, 0.f).GetSafeNormal();

	FVector DropLocation = InDropLocation;
	DropLocation.X += FMath::RandRange(-1000.f, 1000.f);
	DropLocation.Y += FMath::RandRange(-1000.f, 1000.f);

	FVector Start = DropLocation - Direction * TotalDistance / 2;
	FVector End = DropLocation + Direction * TotalDistance / 2;

	SetActorRotation(Direction.Rotation());
	SetActorLocation(Start);

	ProjectileMovement->InitialSpeed = Speed;
	ProjectileMovement->MaxSpeed = Speed;
	ProjectileMovement->Velocity = Direction * Speed;

	bIsDropped = false;
	CurrentDistance = 0.f;
	ProjectileMovement->Activate();
}

void ABATransportShip::HandleDropFromPlane()
{
	DropFromPlane.Broadcast();
}

void ABATransportShip::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (HasAuthority())
	{
		CurrentDistance += DeltaTime * Speed;

		if (!bIsDropped && CurrentDistance >= TotalDistance / 2)
		{
			bIsDropped = true;
			
			if (bIsPlayer)
			{
				HandleDropFromPlane();
			}
			else
			{
				SpawnItemBox();
			}
		}

		if (CurrentDistance >= TotalDistance)
		{
			Destroy();
		}
	}
}

void ABATransportShip::SpawnItemBox()
{
	if (!HasAuthority()) return;
	if (!Item) return;

	FVector DropLocation = GetActorLocation();
	DropLocation += FVector(0.f, 0.f, -100.f);

	ABAItemBox* Box = GetWorld()->SpawnActor<ABAItemBox>(
		ItemBox,
		DropLocation,
		FRotator::ZeroRotator
	);

	if (Box)
	{
		Box->SetItem(Item);
	}
}



