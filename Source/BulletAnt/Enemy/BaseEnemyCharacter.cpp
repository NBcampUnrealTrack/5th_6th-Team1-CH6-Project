// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseEnemyCharacter.h"
#include "Components/StateTreeComponent.h"
#include "Enemy/Spawn/SpawnManagerSubsystem.h"

ABaseEnemyCharacter::ABaseEnemyCharacter()
{
	// Data Asset이나 SpawnerManager가 할당해주기
	AcceptanceRadius = 100.0f;
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	
	PrimaryActorTick.bCanEverTick = false;
	
	StateTreeComponent = CreateDefaultSubobject<UStateTreeComponent>(TEXT("StateTreeComponent"));
}

AActor* ABaseEnemyCharacter::GetTargetActor() const
{
	return TargetActor;
}

void ABaseEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	UE_LOG(LogTemp, Error, TEXT("Begin"));
	
	UWorld* World = GetWorld();
	if (IsValid(World))
	{
		
		USpawnManagerSubsystem* SpawnManagerSubsystem = GetWorld()->GetSubsystem<USpawnManagerSubsystem>();
		if (IsValid(SpawnManagerSubsystem))
		{
			TargetActor = SpawnManagerSubsystem->GetTargetActor();			
		}
		else
		{
			TargetActor = nullptr;
		}
	}
	OnTargetActor.Broadcast();
}

