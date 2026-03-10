#include "Shop/BAItemBox.h"

#include "Player/BACharacter.h"

ABAItemBox::ABAItemBox()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(Root);
	Mesh->SetSimulatePhysics(true);
}

void ABAItemBox::Use_Implementation(AActor* User)
{
	ABACharacter* Player = Cast<ABACharacter>(User);
	if (!Player) return;

	Player->OwnedEquipment[1] = Item;
}

void ABAItemBox::SetItem(TSubclassOf<AActor> InItem)
{
	Item = InItem;
}

void ABAItemBox::BeginPlay()
{
	Super::BeginPlay();
	
}


