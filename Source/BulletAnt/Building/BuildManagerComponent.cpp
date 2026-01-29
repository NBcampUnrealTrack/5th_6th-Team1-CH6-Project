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

    // 프리뷰 이동/회전
    FVector Location;
    FRotator Rotation;
    bool bHasValidSurface = false;   
    
    if (!ComputePreviewPlacement(Location, Rotation, bHasValidSurface))
    {
        PreviewActor->SetCanPlace(false);
        return;
    }

    PreviewActor->UpdateTransform(Location, Rotation);

    // 설치 가능 판정
    bool bCanPlace;
    if (bHasValidSurface)
    {
        bCanPlace = CheckCanPlaceAt(Location, Rotation, PreviewActor->GetBuildingBoxExtent());
    }
    else
    {
        bCanPlace = false;
    }
    PreviewActor->SetCanPlace(bCanPlace);
}

void UBuildManagerComponent::EnterBuildMode()
{
    if (bBuildMode || !BuildingTable || !PreviewActorClass)
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

    bBuildMode = true;
    SetCurrentBuildingRow("TestTurret");
    
    RefreshCachedRef();
    CurrentYaw = 0.f;

    SetComponentTickEnabled(true);
}

void UBuildManagerComponent::ExitBuildMode()
{
    bBuildMode = false;

    SetComponentTickEnabled(false);
    CachedOwner = nullptr;
    CachedPC = nullptr;
    CachedBuildingRow = nullptr;
    CurrentYaw = 0.f;

    if (PreviewActor)
    {
        PreviewActor->Destroy();
        PreviewActor = nullptr;
    }
}

void UBuildManagerComponent::TryPlace()
{
    if (!bBuildMode)
    {
        return;
    }

    if (!PreviewActor || !PreviewActor->CanPlace())
    {
        return;
    }

    ServerTryPlace(CurrentBuildingRow, PreviewActor->GetActorLocation(), PreviewActor->GetActorRotation());
}

void UBuildManagerComponent::RotatePreviewByWheel(float WheelAxisValue)
{
    if (!bBuildMode || !PreviewActor)
    {
        return;
    }

    if (FMath::IsNearlyZero(WheelAxisValue))
    {
        return;
    }

    CurrentYaw = FMath::Fmod(CurrentYaw + FMath::Sign(WheelAxisValue) * WheelYawStep, 360.f);
    if (CurrentYaw < 0.f) 
    {
        CurrentYaw += 360.f;
    }
}

bool UBuildManagerComponent::ComputePreviewPlacement(FVector& OutLocation, FRotator& OutRotation, bool& bOutHasValidSurface)
{
    UWorld* World = GetWorld();
    AActor* OwnerActor = CachedOwner.Get();
    APlayerController* PC = CachedPC.Get();

    if (!World || !OwnerActor || !PC)
    {
        return false;
    }

    // 화면 중앙
    int32 SizeX = 0;
    int32 SizeY = 0;
    PC->GetViewportSize(SizeX, SizeY);

    FVector WorldPos;
    FVector WorldDir;
    if (!PC->DeprojectScreenPositionToWorld(SizeX * 0.5f, SizeY * 0.5f, WorldPos, WorldDir))
    {
        return false;
    }

    // 라인트레이스
    const float MaxDist = 1000.f;
    const FVector Start = WorldPos;
    const FVector End = Start + WorldDir * MaxDist;

    FHitResult Hit;
    FCollisionQueryParams Params(SCENE_QUERY_STAT(BuildTrace), false);
    Params.AddIgnoredActor(OwnerActor);

    if (World->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
    {
        OutLocation = Hit.ImpactPoint;
        bOutHasValidSurface = true;

        // 디버그 스피어
        DrawDebugSphere(GetWorld(), Hit.ImpactPoint, 12.f, 12, FColor::Yellow, false, 0.f);
    }
    else
    {
        OutLocation = End;
        bOutHasValidSurface = false;
    }

    const float BaseYaw = PC->GetControlRotation().Yaw;
    OutRotation = FRotator(0.f, BaseYaw + CurrentYaw, 0.f);

    return true;
}

void UBuildManagerComponent::ServerTryPlace_Implementation(FName BuildingRow, const FVector& Location, const FRotator& Rotation)
{
    if (!BuildingTable)
    {
        return;
    }

    const FBuildingRow* Row = BuildingTable->FindRow<FBuildingRow>(BuildingRow, TEXT("ServerTryPlace"));
    if (!Row || !Row->BuildingClass)
    {
        return;
    }

    if (!CheckCanPlaceAt(Location, Rotation, Row->BuildingBoxExtent))
    {
        return;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    FActorSpawnParameters Params;
    Params.Owner = GetOwner();
    Params.Instigator = Cast<APawn>(GetOwner());
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    ABaseBuilding* Spawned = World->SpawnActor<ABaseBuilding>(Row->BuildingClass, Location, Rotation, Params);
    if (Spawned)
    {
        Spawned->SetBuildingBoxExtent(Row->BuildingBoxExtent);
    }
}

bool UBuildManagerComponent::CheckCanPlaceAt(const FVector& Location, const FRotator& Rotation, const FVector& InBoxExtent) const
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }

    FVector BoxExtent = InBoxExtent;
    BoxExtent.X = FMath::Max(10.f, BoxExtent.X);
    BoxExtent.Y = FMath::Max(10.f, BoxExtent.Y);
    BoxExtent.Z = FMath::Max(10.f, BoxExtent.Z);

    FCollisionQueryParams Params(SCENE_QUERY_STAT(BuildOverlap), false);
    if (PreviewActor) 
    {
        Params.AddIgnoredActor(PreviewActor);
    }

    FCollisionObjectQueryParams ObjParams;
    ObjParams.AddObjectTypesToQuery(ECC_WorldStatic);
    ObjParams.AddObjectTypesToQuery(ECC_WorldDynamic);
    ObjParams.AddObjectTypesToQuery(ECC_Pawn);

    TArray<FOverlapResult> Overlaps;
    const bool bAnyOverlap = World->OverlapMultiByObjectType(
        Overlaps,
        Location + FVector(0, 0, BoxExtent.Z),
        Rotation.Quaternion(),
        ObjParams,
        FCollisionShape::MakeBox(BoxExtent),
        Params
    );

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

void UBuildManagerComponent::SetCurrentBuildingRow(FName NewRow)
{
    if (!BuildingTable) 
    {
        return;
    }

    const FBuildingRow* Row = BuildingTable->FindRow<FBuildingRow>(NewRow, TEXT("SetCurrentBuildingRow"));
    if (!Row) 
    {
        return;
    }

    CurrentBuildingRow = NewRow;
    CachedBuildingRow = Row;

    if (PreviewActor)
    {
        PreviewActor->InitWithData(*CachedBuildingRow);
    }
}

