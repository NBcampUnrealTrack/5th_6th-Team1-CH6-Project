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
	if (!IsValid(this) || GetWorld()->bIsTearingDown)	// 게임 강제 종료시 발생하는 에러 예방 코드
	{
		return;
	}

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
