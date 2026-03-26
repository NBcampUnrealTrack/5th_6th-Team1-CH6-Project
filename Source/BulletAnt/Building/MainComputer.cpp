// Fill out your copyright notice in the Description page of Project Settings.


#include "Building/MainComputer.h"
#include "Multiplayer/MultiplayerSubsystem.h"

void AMainComputer::GetInteractionOptions_Implementation(AActor* User, TArray<FInteractionOption>& OutOptions) const
{
	OutOptions.Reset();

	FInteractionOption Op1;
	Op1.Key = EKeys::F;
	Op1.Label = FText::FromString(TEXT("시작"));
	Op1.ActionName = TEXT("Start");
	OutOptions.Add(Op1);
}

void AMainComputer::Interaction_Implementation(AActor* User, FName ActionName)
{
	if (ActionName == TEXT("Start"))
	{
		UMultiplayerSubsystem* MultiplayerSubsystem = GetGameInstance()->GetSubsystem<UMultiplayerSubsystem>();
		if (IsValid(MultiplayerSubsystem) == true)
		{
			MultiplayerSubsystem->ServerTravelToLevel("/Game/SpaceBase/Maps/MainLevel");
		}
	}
}