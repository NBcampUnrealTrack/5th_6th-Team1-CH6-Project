// Fill out your copyright notice in the Description page of Project Settings.

#include "BaseEnemyCharacter.h"
#include "Components/StateTreeComponent.h"
#include "Enemy/Spawn/SpawnManagerSubsystem.h"

ABaseEnemyCharacter::ABaseEnemyCharacter()
{
	AcceptanceRadius = 100.0f;
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	PrimaryActorTick.bCanEverTick = false;

	StateTreeComponent = CreateDefaultSubobject<UStateTreeComponent>(TEXT("StateTreeComponent"));
}

void ABaseEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();

	// SpawnManagerSubsystem에서 TargetActor 가져오기
	if (UWorld* World = GetWorld())
	{
		if (USpawnManagerSubsystem* SpawnManagerSubsystem = World->GetSubsystem<USpawnManagerSubsystem>())
		{
			TargetActor = SpawnManagerSubsystem->GetTargetActor();
		}
		else
		{
			// Subsystem이 없으면 직접 플레이어 찾기
			if (APlayerController* PC = World->GetFirstPlayerController())
			{
				TargetActor = PC->GetPawn();
			}
		}
	}

	if (!IsValid(TargetActor))
	{
		UE_LOG(LogTemp, Warning, TEXT("BaseEnemyCharacter: TargetActor not found"));
	}
}
