// Fill out your copyright notice in the Description page of Project Settings.


#include "Building/BuildManagerComponent.h"
#include "Building/BuildPreview.h"
#include "Building/BuildManagerComponent.h"
#include "Building/BaseBuilding.h"

UBuildManagerComponent::UBuildManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UBuildManagerComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UBuildManagerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!bBuildMode || !PreviewActor) 
    {
        return;
    }

    AActor* OwnerActor = GetOwner();
    if (!OwnerActor)
    {
        return;
    }
    APawn* PawnOwner = Cast<APawn>(OwnerActor);
    if (!PawnOwner) 
    {
        return;
    }
    APlayerController* PC = Cast<APlayerController>(PawnOwner->GetController());
    if (!PC) 
    {
        return;
    }

    // 화면 중앙
    int32 SizeX = 0, SizeY = 0;
    PC->GetViewportSize(SizeX, SizeY);

    FVector WorldPos, WorldDir;
    const bool bDeprojectOK = PC->DeprojectScreenPositionToWorld(
        SizeX * 0.5f,
        SizeY * 0.5f,
        WorldPos,
        WorldDir
    );
    if (!bDeprojectOK) 
    {
        return;
    }

    const FVector Start = WorldPos;
    const FVector End = Start + WorldDir * 5000.f;

    // 라인트레이스
    FHitResult Hit;
    FCollisionQueryParams Params(SCENE_QUERY_STAT(BuildTrace), false);
    Params.AddIgnoredActor(OwnerActor);

    const bool bHit = GetWorld()->LineTraceSingleByChannel(
        Hit,
        Start,
        End,
        ECC_Visibility,
        Params
    );

    // 디버그 스피어
    DrawDebugSphere(GetWorld(), Hit.ImpactPoint, 12.f, 12, FColor::Yellow, false, 0.f);

    // 프리뷰 이동/회전
    const FVector Location = Hit.ImpactPoint;

    const float Yaw = PC->GetControlRotation().Yaw;
    const FRotator Rotation(0.f, Yaw, 0.f);

    PreviewActor->UpdateTransform(Location, Rotation);

    // 설치 가능 판정
    const bool bCanPlace = CheckCanPlaceAt(Location, PreviewActor->GetPlacementRadius());
    PreviewActor->SetCanPlace(bCanPlace);
}

void UBuildManagerComponent::EnterBuildMode()
{
    if (bBuildMode || !DefaultBuildData || !PreviewActorClass)
    {
        return;
    }

    bBuildMode = true;
    CurrentData = DefaultBuildData;

    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    PreviewActor = World->SpawnActor<ABuildPreview>(PreviewActorClass);
    if (PreviewActor)
    {
        PreviewActor->InitWithData(CurrentData);
    }
}

void UBuildManagerComponent::ExitBuildMode()
{
    bBuildMode = false;
    CurrentData = nullptr;

    if (PreviewActor)
    {
        PreviewActor->Destroy();
        PreviewActor = nullptr;
    }
}

void UBuildManagerComponent::TryPlace()
{
    if (!bBuildMode || !PreviewActor || !CurrentData) 
    {
        return;
    }

    if (!PreviewActor->CanPlace())
    {
        return;
    }

    if (!CurrentData->BuildingClass)
    {
        return;
    }

    UWorld* World = GetWorld();
    if (!World) 
    {
        return;
    }

    const FVector SpawnLoc = PreviewActor->GetActorLocation();
    const FRotator SpawnRot = PreviewActor->GetActorRotation();

    FActorSpawnParameters Params;
    Params.Owner = GetOwner();
    Params.Instigator = Cast<APawn>(GetOwner());
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    AActor* Spawned = World->SpawnActor<AActor>(CurrentData->BuildingClass, SpawnLoc, SpawnRot, Params);
    if (!Spawned) 
    {
        return;
    }
}

bool UBuildManagerComponent::CheckCanPlaceAt(const FVector& Location, float Radius) const
{
    // 임시
    return true;
}

