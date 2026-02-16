// Fill out your copyright notice in the Description page of Project Settings.


#include "Building/BaseCore.h"
#include "Framework/BAGameState.h"

void ABaseCore::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		UWorld* World = GetWorld();
		if (IsValid(World))
		{
			if (ABAGameState* GS = World->GetGameState<ABAGameState>())
			{
				GS->SetTargetCore(this);
			}
		}
	}
}
