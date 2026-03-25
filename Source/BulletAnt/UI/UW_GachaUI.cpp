#include "UI/UW_GachaUI.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "Framework/BAGameState.h"

void UUW_GachaUI::NativeConstruct()
{
	Super::NativeConstruct();

	if (UpButton)
	{
		UpButton->OnClicked.AddDynamic(this, &UUW_GachaUI::HadleUpButtonClicked);
	}

	if (DownButton)
	{
		DownButton->OnClicked.AddDynamic(this, &UUW_GachaUI::HandleDownButtonClicked);
	}

	if (OkayButton)
	{
		OkayButton->OnClicked.AddDynamic(this, &UUW_GachaUI::HandleOkayButtonClicked);
	}

	ABAGameState* GS = Cast<ABAGameState>(GetWorld()->GetGameState());
	if (IsValid(GS))
	{
		FOnOreChanged::FDelegate Delegate;
		Delegate.BindDynamic(this, &UUW_GachaUI::SetOreCount);
		GS->BindOnOreChanged(Delegate);

		const auto& OreInventory = GS->GetOreInventory();
		for (const auto& OrePair : OreInventory)
		{
			SetOreCount(OrePair.Key, OrePair.Value);
		}
	}
}

void UUW_GachaUI::SetOreCount(EOreType Ore, int32 Count)
{
	if (!GoldText || !MineralText) return;

	switch (Ore)
	{
	case EOreType::Gold :
		GoldText->SetText(FText::AsNumber(Count));
		break;
	case EOreType::Mineral :
		MineralText->SetText(FText::AsNumber(Count));
		break;
	default:
		break;
	}
}

void UUW_GachaUI::InitGachaUI(TMap<EOreType, int32> InCost)
{
	for (auto Cost : InCost)
	{
		switch (Cost.Key)
		{
		case EOreType::Gold :
			RequireGoldText->SetText(FText::AsNumber(Cost.Value));
			RequireGold = Cost.Value;
			break;
		case EOreType::Mineral :
			RequireMineralText->SetText(FText::AsNumber(Cost.Value));
			RequireMineral = Cost.Value;
			break;

		default:
			break;
		}
	}

	GachaCount = 0;
	CountText->SetText(FText::AsNumber(GachaCount));
	RequireGoldText->SetText(FText::AsNumber(GachaCount * RequireGold));
	RequireMineralText->SetText(FText::AsNumber(GachaCount * RequireMineral));
}

void UUW_GachaUI::HadleUpButtonClicked()
{
	GachaCount = FMath::Clamp(GachaCount + 1, 0, 100);
	CountText->SetText(FText::AsNumber(GachaCount));
	RequireGoldText->SetText(FText::AsNumber(GachaCount * RequireGold));
	RequireMineralText->SetText(FText::AsNumber(GachaCount * RequireMineral));
}

void UUW_GachaUI::HandleDownButtonClicked()
{
	GachaCount = FMath::Clamp(GachaCount - 1, 0, 100);
	CountText->SetText(FText::AsNumber(GachaCount));
	RequireGoldText->SetText(FText::AsNumber(GachaCount * RequireGold));
	RequireMineralText->SetText(FText::AsNumber(GachaCount * RequireMineral));
}

void UUW_GachaUI::HandleOkayButtonClicked()
{
	OnOkayButtonClicked.Broadcast(GachaCount);
	CountText->SetText(FText::AsNumber(GachaCount));
}
