// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Egg/EggCharacter.h"
#include "GAS/BAGameplayTags.h"
#include "AbilitySystemComponent.h"
#include "Enemy/Spawn/SpawnManagerSubsystem.h"

bool AEggCharacter::ShouldCallAfterAttack()
{
	return true;
}

void AEggCharacter::AfterAttack()
{
	Destroy();

	UWorld* World = GetWorld();
	if (IsValid(World))
	{
		USpawnManagerSubsystem* SpawnManagerSubsystem = GetWorld()->GetSubsystem<USpawnManagerSubsystem>();
		if (IsValid(SpawnManagerSubsystem))
		{
			SpawnManagerSubsystem->OnEnemyDie();
		}
	}
}
