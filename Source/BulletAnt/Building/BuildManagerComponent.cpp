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
    FVector FreeLocation;
    FRotator Rotation;
    bool bHasValidSurface = false;   
    
    if (!ComputePreviewPlacement(FreeLocation, Rotation, bHasValidSurface))
    {
        PreviewActor->SetCanPlace(false);
        return;
    }

    // 스냅 체크
    PreviewActor->UpdateTransform(FreeLocation, Rotation);
    FVector FinalLocation = FreeLocation;
    if (bSnapMode)
    {
        TrySnapPreview(FinalLocation);
    }

    PreviewActor->UpdateTransform(FinalLocation, Rotation);
    PreviewActor->DrawSnapPointsDebug(false, 0.02f);

    // 설치 가능 판정
    bool bCanPlace;
    if (bHasValidSurface)
    {
        bCanPlace = CheckCanPlaceAt(FinalLocation, Rotation, PreviewActor->GetBuildingBoxExtent());
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
    SetCurrentBuildingRow(DefaultBuildingRow);
    
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

void UBuildManagerComponent::ToggleSnapMode()
{
    bSnapMode = !bSnapMode;
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
    const FVector Start = WorldPos;
    const FVector End = Start + WorldDir * MaxBuildDistance;

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

bool UBuildManagerComponent::TrySnapPreview(FVector& InOutLocation) const
{
    UWorld* World = GetWorld();
    if (!World || !PreviewActor)
    {
        return false;
    }

    // 주변 빌딩 후보 찾기 (차후에 빌딩 채널로 빌딩들만 체크할 예정)
    FCollisionQueryParams Params(SCENE_QUERY_STAT(SnapSearch), false);
    Params.AddIgnoredActor(PreviewActor);

    FCollisionObjectQueryParams ObjParams;
    ObjParams.AddObjectTypesToQuery(ECC_WorldDynamic);
    ObjParams.AddObjectTypesToQuery(ECC_WorldStatic);

    TArray<FOverlapResult> Overlaps;
    const bool bAnyOverlap = World->OverlapMultiByObjectType(
        Overlaps,
        InOutLocation,
        FQuat::Identity,
        ObjParams,
        FCollisionShape::MakeSphere(SnapSearchRadius),
        Params
    );

    if (!bAnyOverlap)
    {
        return false;
    }

    // 프리뷰 스냅 포인트들
    TArray<FVector> PreviewSnapPoints;
    PreviewActor->GetSnapPointsWorld(PreviewSnapPoints);

    float BestDistSq = SnapMaxDistance * SnapMaxDistance;
    FVector BestDelta = FVector::ZeroVector;
    bool bFound = false;

    // 각 후보 빌딩의 스냅 포인트와 매칭
    for (const FOverlapResult& R : Overlaps)
    {
        ABaseBuilding* OtherBuilding = Cast<ABaseBuilding>(R.GetActor());
        if (!OtherBuilding)
        {
            continue;
        }

        TArray<FVector> OtherSnapPoints;
        OtherBuilding->GetSnapPointsWorld(OtherSnapPoints);

        for (const FVector& PPrev : PreviewSnapPoints)
        {
            for (const FVector& POther : OtherSnapPoints)
            {
                FVector Delta = POther - PPrev;

                const float DistSq = Delta.SizeSquared();
                if (DistSq < BestDistSq)
                {
                    BestDistSq = DistSq;
                    BestDelta = Delta;
                    bFound = true;
                }
            }
        }
    }

    if (!bFound)
    {
        return false;
    }

    // 적용
    InOutLocation += BestDelta;
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
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    ABaseBuilding* Spawned = World->SpawnActor<ABaseBuilding>(Row->BuildingClass, Location, Rotation, Params);
    if (Spawned)
    {
        Spawned->SetBuildingBoxExtent(Row->BuildingBoxExtent);
        Spawned->DrawSnapPointsDebug(true, -1.f);
    }
}

bool UBuildManagerComponent::CheckCanPlaceAt(const FVector& Location, const FRotator& Rotation, const FVector& InBoxExtent) const
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }

    FVector BoxExtent = InBoxExtent.ComponentMax(FVector(10.f));
    const FVector Center = Location + FVector(0, 0, BoxExtent.Z);
    const FQuat RotQ = Rotation.Quaternion();
    const FCollisionShape BoxShape = FCollisionShape::MakeBox(BoxExtent);

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
        Center,
        RotQ,
        ObjParams,
        BoxShape,
        Params
    );

    if (!bAnyOverlap)
    {
        return true;
    }

    for (const FOverlapResult& R : Overlaps)
    {
        AActor* Other = R.GetActor();
        UPrimitiveComponent* OtherComp = R.GetComponent();

        if (!Other || !OtherComp)
        {
            continue;
        }

        if (Other->ActorHasTag(GroundActorTag)) 
        {
            continue;
        }

        // 작은 오차의 겹침은 허용
        FMTDResult MTD;
        const bool bHasMTD = OtherComp->ComputePenetration(MTD, BoxShape, Center, RotQ);
        if (bHasMTD && MTD.Distance > AllowedPenetrationDistance)
        {
            return false;
        }
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

