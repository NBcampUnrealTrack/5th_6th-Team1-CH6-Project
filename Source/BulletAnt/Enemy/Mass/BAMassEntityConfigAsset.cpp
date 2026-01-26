// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Mass/BAMassEntityConfigAsset.h"
#include "MassStateTreeTrait.h"

UBAMassEntityConfigAsset::UBAMassEntityConfigAsset()
{
	DefaultStateTreeTrait = CreateDefaultSubobject<UMassStateTreeTrait>("MassStateTreeTrait");
}

void UBAMassEntityConfigAsset::PostInitProperties()
{
	Super::PostInitProperties();
	
	Config.AddTrait(*DefaultStateTreeTrait);
}
