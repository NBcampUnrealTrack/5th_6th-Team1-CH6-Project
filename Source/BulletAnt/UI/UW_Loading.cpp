#include "UI/UW_Loading.h"
#include "Components/Overlay.h"

void UUW_Loading::ShowLoadingPanel(bool bShow)
{
	if (IsValid(LoadingPanel) == true)
	{
		ESlateVisibility NewVisibility = bShow == true ? ESlateVisibility::Visible : ESlateVisibility::Collapsed;
		LoadingPanel->SetVisibility(NewVisibility);
	}
}
