// Fill out your copyright notice in the Description page of Project Settings.


#include "Building/BuildManagerComponent.h"
#include "Building/BuildPreview.h"
#include "Building/BaseBuilding.h"
#include "Engine/OverlapResult.h"

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

    if (!bBuildMode || !PreviewActor) 
    {
        return;
    }

    PreviewActor->SetCanPlace(false);

    AActor* OwnerActor = CachedOwner.Get();
    APlayerController* PC = CachedPC.Get();
    if (!OwnerActor || !PC)
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

    // If nothing was hit, hide preview or keep last position
    if (!bHit)
    {
        return;
    }

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

    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    PreviewActor = World->SpawnActor<ABuildPreview>(PreviewActorClass);
    if (!PreviewActor)
    {
        return;
    }

    // Set state only after successful spawn
    bBuildMode = true;
    CurrentData = DefaultBuildData;

    RefreshCachedRef();
    SetComponentTickEnabled(true);

    PreviewActor->InitWithData(CurrentData);
}

void UBuildManagerComponent::ExitBuildMode()
{
    bBuildMode = false;
    CurrentData = nullptr;

    SetComponentTickEnabled(false);
    CachedOwner = nullptr;
    CachedPC = nullptr;

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
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }

    const float UseRadius = FMath::Max(5.f, Radius);

    FCollisionQueryParams Params(SCENE_QUERY_STAT(BuildOverlap), false);
    if (PreviewActor) 
    {
        Params.AddIgnoredActor(PreviewActor);
    }
    if (CachedOwner.IsValid())
    {
        Params.AddIgnoredActor(CachedOwner.Get());
    }

    FCollisionObjectQueryParams ObjParams;
    ObjParams.AddObjectTypesToQuery(ECC_WorldStatic);
    ObjParams.AddObjectTypesToQuery(ECC_WorldDynamic);
    ObjParams.AddObjectTypesToQuery(ECC_Pawn);

    TArray<FOverlapResult> Overlaps;
    const bool bAnyOverlap = World->OverlapMultiByObjectType(
        Overlaps,
        Location,
        FQuat::Identity,
        ObjParams,
        FCollisionShape::MakeSphere(UseRadius),
        Params
    );

    DrawDebugSphere(World, Location, UseRadius, 16, bAnyOverlap ? FColor::Red : FColor::Green, false, 0.f, 0, 1.f);

    if (!bAnyOverlap)
    {
        return true;
    }

    for (const FOverlapResult& R : Overlaps)
    {
        AActor* Other = R.GetActor();
        if (!Other) 
        {
            continue;
        }

        if (Other->ActorHasTag("Ground")) 
        {
            continue;
        }

        return false;
    }

    return true;
}

void UBuildManagerComponent::RefreshCachedRef()
{
    CachedOwner = GetOwner();
    APawn* OwnerPawn = Cast<APawn>(CachedOwner.Get());
    CachedPC = OwnerPawn ? Cast<APlayerController>(OwnerPawn->GetController()) : nullptr;
}

