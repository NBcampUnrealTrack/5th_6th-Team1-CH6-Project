// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Egg/EggCharacter.h"
#include "GAS/BAGameplayTags.h"

bool AEggCharacter::ShouldCallAfterAttack()
{
	return true;
}

void AEggCharacter::AfterAttack()
{
	FGameplayEventData Payload;
	Payload.EventMagnitude = 5.f;

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
		this,
		TAG_Event_Combat_Dead,
		Payload
	);
}
