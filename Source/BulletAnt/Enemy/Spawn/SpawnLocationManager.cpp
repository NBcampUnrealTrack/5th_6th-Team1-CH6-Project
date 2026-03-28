// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Spawn/SpawnLocationManager.h"
#include "Components/BoxComponent.h"
#include "Enemy/Spawn/SpawnManagerSubsystem.h"
#include "Engine/TriggerBox.h"

ASpawnLocationManager::ASpawnLocationManager()
{
	PrimaryActorTick.bCanEverTick = false;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
}

void ASpawnLocationManager::BeginPlay()
{
	Super::BeginPlay();

	if (!HasAuthority())
	{
		return;
	}

	if (SpawnBoxes.Num() == 0)
	{
		return;
	}
	for (FSpawnBoxEntry& Entry : SpawnBoxes)
	{
		ATriggerBox* Box = Entry.SpawnBox;
		if (!IsValid(Box))
		{
			continue;
		}
		Box->SetActorTickEnabled(false);

		UShapeComponent* ShapeComp = Box->GetCollisionComponent();
		if (!IsValid(ShapeComp))
		{
			continue;
		}
		ShapeComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		ShapeComp->SetGenerateOverlapEvents(false);
		ShapeComp->SetCollisionResponseToAllChannels(ECR_Ignore);
		ShapeComp->SetCanEverAffectNavigation(false);

		UBoxComponent* BoxComponent = Cast<UBoxComponent>(ShapeComp);
		if (!IsValid(BoxComponent))
		{
			continue;
		}

		Entry.Origin = BoxComponent->GetComponentLocation();
		FVector BoxExtent = BoxComponent->GetScaledBoxExtent();
		Entry.Extent = FVector2D(BoxExtent.X, BoxExtent.Y);
		Entry.Direction = BoxComponent->GetComponentRotation();
		Entry.Weight = BoxExtent.X * BoxExtent.Y;
		TotalWeight += Entry.Weight;
	}

	UWorld* World = GetWorld();
	if (IsValid(World))
	{
		USpawnManagerSubsystem* SpawnManager = GetWorld()->GetSubsystem<USpawnManagerSubsystem>();
		SpawnManager->SetCachedSpawnLocationManager(this);
	}	
}

FVector ASpawnLocationManager::GetRandomSpawnLocation() const
{
	if (!HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("ASpawnLocationManager GetRandomLocationInGroup : Client Call Error"));
		return FVector::ZeroVector;
	}
	if (SpawnBoxes.Num() == 0)
	{
		return GetActorLocation();
	}

	const FSpawnBoxEntry& RandomSpawnBoxEntry = GetRandomSpawnBox();
	ATriggerBox* Box = RandomSpawnBoxEntry.SpawnBox;
	if (!IsValid(Box))
	{
		return GetActorLocation();
	}

	FVector2D BoxExtent = RandomSpawnBoxEntry.Extent;
	FVector RandomPoint = FVector(FMath::FRandRange(-BoxExtent.X, BoxExtent.X),
								  FMath::FRandRange(-BoxExtent.Y, BoxExtent.Y), 
								  0);

	return RandomSpawnBoxEntry.Origin + RandomSpawnBoxEntry.Direction.RotateVector(RandomPoint);
}

const FSpawnBoxEntry& ASpawnLocationManager::GetRandomSpawnBox() const
{
	float RandomWeight = FMath::FRandRange(0.0f, TotalWeight);
	float AccumulatedWeight = 0.0f;

	for (const FSpawnBoxEntry& Entry : SpawnBoxes)
	{
		if (!IsValid(Entry.SpawnBox))
		{
			continue;
		}

		AccumulatedWeight += Entry.Weight;
		if (RandomWeight <= AccumulatedWeight)
		{
			return Entry;
		}
	}

	return SpawnBoxes[0];
}