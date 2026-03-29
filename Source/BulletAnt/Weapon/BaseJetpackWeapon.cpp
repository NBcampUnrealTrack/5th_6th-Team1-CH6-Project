#include "Weapon/BaseJetpackWeapon.h"

#include "Weapon/Data/JetpackWeaponDataAsset.h"
#include "NiagaraComponent.h"
#include "Net/UnrealNetwork.h"

ABaseJetpackWeapon::ABaseJetpackWeapon()
{
    LeftFlame = CreateDefaultSubobject<UNiagaraComponent>(TEXT("LeftFlame"));
    RightFlame = CreateDefaultSubobject<UNiagaraComponent>(TEXT("RightFlame"));

    LeftFlame->SetupAttachment(GetWeaponMesh());
    RightFlame->SetupAttachment(GetWeaponMesh());

    LeftFlame->bAutoActivate = false;
    RightFlame->bAutoActivate = false;
}

void ABaseJetpackWeapon::OnRep_bJetpackActive()
{
    if (bJetpackActive)
    {
        LeftFlame->Activate();
        RightFlame->Activate();
    }
    else
    {
        LeftFlame->Deactivate();
        RightFlame->Deactivate();
    }
}

void ABaseJetpackWeapon::SetbJetpackActive(bool InbJetpackActive)
{
    bJetpackActive = InbJetpackActive;
}

void ABaseJetpackWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ThisClass, bJetpackActive);
}

void ABaseJetpackWeapon::BeginPlay()
{
	Super::BeginPlay();
	UJetpackWeaponDataAsset* Data = Cast<UJetpackWeaponDataAsset>(WeaponData);

	bAutoActive = Data->bAutoActive;
}

