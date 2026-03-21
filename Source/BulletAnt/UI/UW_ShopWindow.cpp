#include "UI/UW_ShopWindow.h"

#include "Components/Button.h"
#include "Components/VerticalBox.h"
#include "UI/UW_WeaponButton.h"
#include "Framework/BAGameState.h"
#include "Weapon/BaseWeapon.h"
#include "Player/BAPlayerController.h"
#include "UI/UISubsystem.h"
#include "Building/BaseShop.h"
#include "Player/BACharacter.h"
#include "UI/UW_GachaUI.h"
#include "Kismet/GameplayStatics.h"
#include "UI/UW_WeaponSelectUI.h"
#include "Weapon/Data/WeaponDataAsset.h"
#include "UI/UW_WeaponInfoUI.h"
#include "Weapon/BaseRangedWeapon.h"
#include "Weapon/Data/RangedWeaponDataAsset.h"

void UUW_ShopWindow::OnClickGachaButton(int32 Count)
{
	if (!CachedShop.IsValid())
	{
		OnClickEndButton();
		return;
	}

	ABAPlayerController* PC = Cast<ABAPlayerController>(GetOwningPlayer());
	if (CachedShop->CanBuyGacha(PC, 0, Count))
	{
		UGameplayStatics::PlaySound2D(GetOwningPlayer(), BuySuccessSound);
	}
	else
	{
		UGameplayStatics::PlaySound2D(GetOwningPlayer(), BuyFailedSound);
	}
}

void UUW_ShopWindow::OnClickEndButton()
{
	ABAPlayerController* PC = Cast<ABAPlayerController>(GetOwningPlayer());
	ULocalPlayer* LP = GetOwningLocalPlayer();
	if (LP)
	{
		UUISubsystem* UISubsystem = LP->GetSubsystem<UUISubsystem>();
		if (UISubsystem) 
		{
			UISubsystem->HideUI(EUIType::Shop);
			UISubsystem->ApplyGameOnlyInputMode();
		}
	}
}

void UUW_ShopWindow::HandleWeaponSelected(TSubclassOf<ABaseWeapon> WeaponClass)
{
	if (!WeaponClass) return;

	const ABaseRangedWeapon* Weapon = Cast<ABaseRangedWeapon>(WeaponClass->GetDefaultObject());
	if (Weapon)
	{
		URangedWeaponDataAsset* Data = Cast<URangedWeaponDataAsset>(Weapon->GetWeaponData());
		if (Data)
		{
			WeaponInfoUI->InitWeaponInfoUI(WeaponClass, Data);
			WeaponInfoUI->EquipButton->OnClicked.Clear();
			
			WeaponInfoUI->SetVisibility(ESlateVisibility::Visible);
			WeaponInfoUI->EquipButton->OnClicked.AddDynamic(this, &UUW_ShopWindow::RequestEquipWeapon);
		}
	}
}

void UUW_ShopWindow::SetupWeaponButton()
{
	ABAGameState* GS = Cast<ABAGameState>(GetWorld()->GetGameState());
	if (!GS) return;

	const TArray<TSubclassOf<ABaseWeapon>>& WeaponArray = GS->GetHaveWeaponArray();

	if (WeaponArray.Num() <= 0) return;

	CreateWeaponButton(WeaponArray);
}

void UUW_ShopWindow::InitShopUI(ABaseShop* InShop)
{
	CachedShop = InShop;
	TMap<int32, TMap<EOreType, int32>> Cost = CachedShop->GetCachedCostData();
	GachaUI->InitGachaUI(Cost[0]);
	
	SetupWeaponButton();
	WeaponInfoUI->SetVisibility(ESlateVisibility::Hidden);
}

void UUW_ShopWindow::CreateWeaponButton(const TArray<TSubclassOf<ABaseWeapon>>& Weapons)
{
	if (!WeaponSelectUI->WeaponList || !WeaponButtonClass) return;

	WeaponSelectUI->WeaponList->ClearChildren();

	for (auto WeaponClass : Weapons)
	{
		UUW_WeaponButton* Button = CreateWidget<UUW_WeaponButton>(GetOwningPlayer(), WeaponButtonClass);

		Button->WeaponClass = WeaponClass;

		Button->OnClickWeapon.AddDynamic(this, &UUW_ShopWindow::HandleWeaponSelected);

		ABaseWeapon* CDOWeapon = Cast<ABaseWeapon>(WeaponClass->GetDefaultObject());
		if (CDOWeapon)
		{
			FText WeaponName = CDOWeapon->GetWeaponData()->WeaponName;
			Button->SetWeaponName(WeaponName);
			WeaponSelectUI->WeaponList->AddChild(Button);
		}
	}
}

void UUW_ShopWindow::RequestEquipWeapon()
{
	TSubclassOf<ABaseWeapon> WeaponClass = WeaponInfoUI->WeaponClass;
	if (!WeaponClass) return;

	ABACharacter* Player = Cast<ABACharacter>(GetOwningPlayerPawn());
	if (!Player) return;

	Player->ShowAmmo();
	Player->Server_SetChangeWeapon(WeaponClass, 0);
}

void UUW_ShopWindow::NativeConstruct()
{
	Super::NativeConstruct();

	if (GachaUI)
	{
		GachaUI->OnOkayButtonClicked.AddDynamic(this, &UUW_ShopWindow::OnClickGachaButton);
	}

	if (EndButton)
	{
		EndButton->OnClicked.AddDynamic(this, &UUW_ShopWindow::OnClickEndButton);
	}
}
