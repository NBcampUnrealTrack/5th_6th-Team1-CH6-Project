// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Spawn/LevelSpawner.h"
#include "Components/BoxComponent.h"
#include "Enemy/DataAsset/LevelSpanwerDataAsset.h"
#include "Enemy/BaseEnemy/BaseEnemyCharacter.h"
#include "Enemy/DataAsset/TribeDataAsset.h"
#include "Player/BACharacter.h"

ALevelSpawner::ALevelSpawner()
{
	PrimaryActorTick.bCanEverTick = false;

	DetectionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("DetectionBox"));
	SetRootComponent(DetectionBox);

	SpawnBox = CreateDefaultSubobject<UBoxComponent>(TEXT("SpawnBox"));
	SpawnBox->SetupAttachment(RootComponent);
	SpawnBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ALevelSpawner::BeginPlay()
{
	Super::BeginPlay();
	
	if (HasAuthority())
	{
		DetectionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		DetectionBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);	// Player
		DetectionBox->OnComponentBeginOverlap.AddDynamic(this, &ALevelSpawner::OnDetectionBoxBeginOverlap);
		DetectionBox->OnComponentEndOverlap.AddDynamic(this, &ALevelSpawner::OnDetectionBoxEndOverlap);

		FVector SpawnBoxExtent = SpawnBox->GetScaledBoxExtent();
		float SpawnSize = SpawnBoxExtent.X * SpawnBoxExtent.Y;
		EnemyCount = SpawnDataAsset->AreaForPerSpawn ? SpawnSize / SpawnDataAsset->AreaForPerSpawn : SpawnSize;
	}
	else
	{
		SpawnBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

void ALevelSpawner::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearAllTimersForObject(this);

	Super::EndPlay(EndPlayReason);
}

void ALevelSpawner::OnDetectionBoxBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!HasAuthority())
	{
		return;
	}
	if (!ensureMsgf(IsValid(SpawnDataAsset), TEXT("ALevelSpawner OnDetectionBoxBeginOverlap : DataAsset Error")))
	{
		return;
	}

	ABACharacter* Target = Cast<ABACharacter>(OtherActor);
	if (IsValid(Target) && TargetActors.Contains(Target) == false)
	{
		TargetActors.Add(Target);
	}

	if (!bAlreadyActivated)
	{
		bAlreadyActivated = true;
		SpawnEnemy();
		GetWorldTimerManager().SetTimer(SpawnIntervalTimerHandle, this, &ALevelSpawner::SpawnEnemy, 1.f, true);
	}
}

void ALevelSpawner::OnDetectionBoxEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!HasAuthority())
	{
		return;
	}

	ABACharacter* Target = Cast<ABACharacter>(OtherActor);
	if (IsValid(Target))
	{
		TargetActors.Remove(Target);
	}
}

void ALevelSpawner::SpawnEnemy()
{
	if (!HasAuthority())
	{
		return;
	}
	if (EnemyCount == 0)
	{
		GetWorldTimerManager().ClearAllTimersForObject(this);
		DetectionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		return;
	}

	FVector SpawnLocation = GetRandomLocation();
	TSubclassOf<ABaseEnemyCharacter> EnemyClass = SelectEnemyClass();
	UTribeDataAsset* TribeDataAsset = SelectTribe();
	ABACharacter* Target = SelectTarget();

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	ABaseEnemyCharacter* Enemy = GetWorld()->SpawnActor<ABaseEnemyCharacter>(
		EnemyClass,
		SpawnLocation,
		FRotator::ZeroRotator,
		SpawnParams
	);

	if (IsValid(Enemy))
	{
		EnemyCount--;
		if (IsValid(TribeDataAsset))
		{
			Enemy->SetTribeType(TribeDataAsset);
			Enemy->OnRep_TribeType();
			Enemy->SetTarget(Target, ETargetPriorityType::Max);
		}
	}
}

FVector ALevelSpawner::GetRandomLocation()
{
	FVector BoxExtent = SpawnBox->GetScaledBoxExtent();
	FVector RandomPoint = FVector(FMath::FRandRange(-BoxExtent.X, BoxExtent.X),
		FMath::FRandRange(-BoxExtent.Y, BoxExtent.Y),
		0);
	return SpawnBox->GetComponentLocation() + SpawnBox->GetComponentRotation().RotateVector(RandomPoint);
}

TSubclassOf<ABaseEnemyCharacter> ALevelSpawner::SelectEnemyClass() const
{
	if (!ensureMsgf(IsValid(SpawnDataAsset), TEXT("ALevelSpawner SelectEnemyClass : DataAsset Error")))
	{
		return nullptr;
	}
	if (!ensureMsgf(SpawnDataAsset->EnemyClass.Num() != 0, TEXT("ALevelSpawner SelectEnemyClass : EnemyClass Error")))
	{
		return nullptr;
	}

	int32 Num = SpawnDataAsset->EnemyClass.Num();
	int32 Idx = FMath::RandRange(0, Num - 1);

	return SpawnDataAsset->EnemyClass[Idx];
}

UTribeDataAsset* ALevelSpawner::SelectTribe() const
{
	if (!ensureMsgf(IsValid(SpawnDataAsset), TEXT("ALevelSpawner SelectEnemyClass : DataAsset Error")))
	{
		return nullptr;
	}
	if (!ensureMsgf(SpawnDataAsset->TribeType.Num() != 0, TEXT("ALevelSpawner SelectEnemyClass : TribeType Error")))
	{
		return nullptr;
	}

	int32 Num = SpawnDataAsset->TribeType.Num();
	int32 Idx = FMath::RandRange(0, Num - 1);

	return SpawnDataAsset->TribeType[Idx];
}

ABACharacter* ALevelSpawner::SelectTarget() const
{
	int32 Num = TargetActors.Num();
	if (Num == 0)
	{
		return nullptr;
	}
	int32 Idx = FMath::RandRange(0, Num - 1);
	if (TargetActors[Idx].IsValid())
	{
		return TargetActors[Idx].Get();
	}
	return nullptr;
}

