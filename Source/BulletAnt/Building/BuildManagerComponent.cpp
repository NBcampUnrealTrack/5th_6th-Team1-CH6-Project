#include "Building/BuildManagerComponent.h"
#include "Building/BuildPreview.h"
#include "Building/BaseBuilding.h"

UBuildManagerComponent::UBuildManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetComponentTickEnabled(false);
}

void UBuildManagerComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UBuildManagerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bBuildMode || !PreviewActor) return;

	AActor* OwnerActor = GetOwner();
	if (!OwnerActor) return;

	APlayerController* PC = CachedPC.Get();
	if (!PC) return;

	int32 SizeX = 0, SizeY = 0;
	PC->GetViewportSize(SizeX, SizeY);

	FVector WorldPos, WorldDir;
	if (!PC->DeprojectScreenPositionToWorld(SizeX * 0.5f, SizeY * 0.5f, WorldPos, WorldDir))
	{
		return;
	}

	const FVector Start = WorldPos;
	const FVector End = Start + WorldDir * 5000.f;

	FHitResult Hit;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(BuildTrace), false);
	Params.AddIgnoredActor(OwnerActor);

	if (!GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
	{
		return;
	}

	DrawDebugSphere(GetWorld(), Hit.ImpactPoint, 12.f, 12, FColor::Yellow, false, 0.f);

	const FVector Location = Hit.ImpactPoint;
	const FRotator Rotation(0.f, PC->GetControlRotation().Yaw, 0.f);

	PreviewActor->UpdateTransform(Location, Rotation);
	PreviewActor->SetCanPlace(CheckCanPlaceAt(Location, PreviewActor->GetPlacementRadius()));
}

void UBuildManagerComponent::EnterBuildMode()
{
	if (bBuildMode || !DefaultBuildData || !PreviewActorClass) return;

	UWorld* World = GetWorld();
	if (!World) return;

	PreviewActor = World->SpawnActor<ABuildPreview>(PreviewActorClass);
	if (!PreviewActor) return;

	bBuildMode = true;
	CurrentData = DefaultBuildData;
	
	RefreshCachedReferences();
	SetComponentTickEnabled(true);

	PreviewActor->InitWithData(CurrentData);
}

void UBuildManagerComponent::ExitBuildMode()
{
	bBuildMode = false;
	CurrentData = nullptr;
	CachedPC = nullptr;
	SetComponentTickEnabled(false);

	if (PreviewActor)
	{
		PreviewActor->Destroy();
		PreviewActor = nullptr;
	}
}

void UBuildManagerComponent::TryPlace()
{
	if (!bBuildMode || !PreviewActor || !CurrentData) return;
	if (!PreviewActor->CanPlace()) return;
	if (!CurrentData->BuildingClass) return;

	UWorld* World = GetWorld();
	if (!World) return;

	FActorSpawnParameters Params;
	Params.Owner = GetOwner();
	Params.Instigator = Cast<APawn>(GetOwner());
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	World->SpawnActor<AActor>(
		CurrentData->BuildingClass,
		PreviewActor->GetActorLocation(),
		PreviewActor->GetActorRotation(),
		Params
	);
}

bool UBuildManagerComponent::CheckCanPlaceAt(const FVector& Location, float Radius) const
{
	return true;
}

void UBuildManagerComponent::RefreshCachedReferences()
{
	APawn* PawnOwner = Cast<APawn>(GetOwner());
	if (!PawnOwner) return;

	CachedPC = Cast<APlayerController>(PawnOwner->GetController());
}

