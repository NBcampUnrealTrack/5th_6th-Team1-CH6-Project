#include "UI/UW_GachaUI.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"

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
}

void UUW_GachaUI::ResetGachaCount()
{
	GachaCount = 0;
	CountText->SetText(FText::AsNumber(GachaCount));
}

void UUW_GachaUI::HadleUpButtonClicked()
{
	GachaCount = FMath::Clamp(GachaCount + 1, 0, 100);
	CountText->SetText(FText::AsNumber(GachaCount));
}

void UUW_GachaUI::HandleDownButtonClicked()
{
	GachaCount = FMath::Clamp(GachaCount - 1, 0, 100);
	CountText->SetText(FText::AsNumber(GachaCount));
}

void UUW_GachaUI::HandleOkayButtonClicked()
{
	OnOkayButtonClicked.Broadcast(GachaCount);
	CountText->SetText(FText::AsNumber(GachaCount));
}
