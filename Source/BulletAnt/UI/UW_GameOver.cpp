// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/UW_GameOver.h"

#include "Player/BAPlayerState.h"
#include "Components/TextBlock.h"
#include "GAS/AttributeSet/EXPAttributeSet.h"
#include "FrameWork/BAGameState.h"

void UUW_GameOver::InitText()
{
	if (ABAPlayerState* PS = Cast<ABAPlayerState>(GetOwningPlayerState()))
	{
		if (KillCountText)
		{
			KillCountText->SetText(FText::AsNumber(PS->GetKillCount()));
		}
		if (DamageText)
		{
			DamageText->SetText(FText::AsNumber(PS->GetTotalDamage()));
		}
		if (LevelText)
		{
			const UEXPAttributeSet* EXPSet = PS->GetAbilitySystemComponent()->GetSet<UEXPAttributeSet>();
			if (EXPSet)
			{
				LevelText->SetText(FText::AsNumber(EXPSet->GetCurrentLevel()));
			}
		}
		if (DaysText)
		{
			UWorld* World = GetWorld();
			if (IsValid(World))
			{
				ABAGameState* GameState = World->GetGameState<ABAGameState>();
				if (IsValid(GameState))
				{
					DaysText->SetText(FText::AsNumber(GameState->GetDate()));
				}
			}
		}
		if (BuildingText)
		{
			BuildingText->SetText(FText::AsNumber(PS->GetBuildCount()));
		}
	}
}
