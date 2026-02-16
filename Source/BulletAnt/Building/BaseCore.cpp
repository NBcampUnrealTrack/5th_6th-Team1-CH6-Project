// Fill out your copyright notice in the Description page of Project Settings.


#include "Building/BaseCore.h"
#include "Enemy/Spawn/SpawnManagerSubsystem.h"

void ABaseCore::BeginPlay()
{
	Super::BeginPlay();

	UWorld* World = GetWorld();
	if (IsValid(World))
	{
		USpawnManagerSubsystem* SpawnManagerSubsystem = GetWorld()->GetSubsystem<USpawnManagerSubsystem>();
		if (IsValid(SpawnManagerSubsystem))
		{
			SpawnManagerSubsystem->SetTargetCore(this);
		}
	}
}
