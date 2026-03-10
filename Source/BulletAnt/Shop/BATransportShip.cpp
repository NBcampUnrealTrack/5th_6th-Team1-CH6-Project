#include "Shop/BATransportShip.h"

#include "Components/SplineComponent.h"
#include "Shop/BAItemBox.h"

ABATransportShip::ABATransportShip()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	PlaneMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlaneMesh"));
	PlaneMesh->SetupAttachment(Root);
	PlaneMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	Spline = CreateDefaultSubobject<USplineComponent>(TEXT("Spline"));
}

void ABATransportShip::InitPlane(FVector& InDropLocation, TSubclassOf<AActor> InItem)
{
	if (!InItem) return;
	Item = InItem;

	float DirectionX = FMath::RandRange(-1.f, 1.f);
	float DirectionY = FMath::RandRange(-1.f, 1.f);
	FVector Direction = FVector(DirectionX, DirectionY, 0.f).GetSafeNormal();
	SetActorRotation(Direction.Rotation());

	FVector DropLocation = InDropLocation;
	DropLocation.X += FMath::RandRange(-1000.f, 1000.f);
	DropLocation.Y += FMath::RandRange(-1000.f, 1000.f);

	FVector Start = DropLocation - Direction * TotalDistance;
	FVector End = DropLocation + Direction * TotalDistance;

	Spline->ClearSplinePoints();

	Spline->AddSplinePoint(Start, ESplineCoordinateSpace::World);
	Spline->AddSplinePoint(DropLocation, ESplineCoordinateSpace::World);
	Spline->AddSplinePoint(End, ESplineCoordinateSpace::World);

	Spline->UpdateSpline();

	SetActorLocation(Start);

	DropDistance = Spline->GetDistanceAlongSplineAtSplinePoint(1);

}

void ABATransportShip::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bIsDropped && CurrentDistance >= DropDistance)
	{
		bIsDropped = true;
		SpawnItemBox();
	}

	CurrentDistance += Speed * DeltaTime;

	float SplineLength = Spline->GetSplineLength();

	FVector Location = Spline->GetLocationAtDistanceAlongSpline(
		CurrentDistance,
		ESplineCoordinateSpace::World
	);

	SetActorLocation(Location);

	if (CurrentDistance >= SplineLength)
	{
		Destroy();
	}
}

void ABATransportShip::SpawnItemBox()
{
	if (!HasAuthority()) return;

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


