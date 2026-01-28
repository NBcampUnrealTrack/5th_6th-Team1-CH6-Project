// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseEnemyCharacter.h"
#include "Components/StateTreeComponent.h"
#include "Enemy/Spawn/SpawnManagerSubsystem.h"

ABaseEnemyCharacter::ABaseEnemyCharacter()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	
	// Data Asset이나 SpawnerManager가 할당해주기
	AcceptanceRadius = 100.0f;
	
	StateTreeComponent = CreateDefaultSubobject<UStateTreeComponent>(TEXT("StateTreeComponent"));
	StateTreeComponent->SetStartLogicAutomatically(false);
	
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

AActor* ABaseEnemyCharacter::GetTargetActor() const
{
	return TargetActor;
}

void ABaseEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	if (HasAuthority())
	{		
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
		
		StateTreeComponent->StartLogic();
	}
}

