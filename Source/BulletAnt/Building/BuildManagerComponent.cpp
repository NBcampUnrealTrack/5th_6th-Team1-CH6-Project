
#include "Building/BuildManagerComponent.h"
#include "Building/BuildPreview.h"
#include "Building/BaseBuilding.h"
#include "Engine/OverlapResult.h"
#include <InputActionValue.h>

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
        TrySnapPreview(FinalLocation, Rotation);
    }

    PreviewActor->UpdateTransform(FinalLocation, Rotation);
    PreviewActor->DrawEdgesDebug(false, 0.02f);

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

    Server_TryPlace(CurrentBuildingRow, PreviewActor->GetActorLocation(), PreviewActor->GetActorRotation());
}

void UBuildManagerComponent::RotatePreviewByWheel(const FInputActionValue& Value)
{
    if (!bBuildMode || !PreviewActor)
    {
        return;
    }

    const float WheelAxisValue = Value.Get<float>();
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

bool UBuildManagerComponent::TrySnapPreview(FVector& InOutLocation, FRotator& InOutRotation)
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

    // 프리뷰 스냅 엣지들
    TArray<FBuildingEdge> PrevEdges;
    PreviewActor->GetEdgesWorld(PrevEdges);

    // 최적 엣지 계산
    float BestScore = TNumericLimits<float>::Max();
    FVector BestDelta = FVector::ZeroVector;
    float BestYaw = InOutRotation.Yaw;
    bool bFound = false;

    for (const FOverlapResult& R : Overlaps)
    {
        ABaseBuilding* OtherBuilding = Cast<ABaseBuilding>(R.GetActor());
        if (!OtherBuilding)
        {
            continue;
        }

        TArray<FBuildingEdge> OtherEdges;
        OtherBuilding->GetEdgesWorld(OtherEdges);
        if (OtherEdges.Num() == 0) 
        {
            continue;
        }

        for (const FBuildingEdge& EO : OtherEdges)
        {
            const FVector2D OtherDir2d = EO.Dir2D();
            if (OtherDir2d.IsNearlyZero())
            {
                continue;
            }

            // 타겟 엣지 yaw (동일방향 / 반대방향)
            const float EdgeYaw = FMath::RadiansToDegrees(FMath::Atan2(OtherDir2d.Y, OtherDir2d.X));
            const float CandidateYaws[2] = { EdgeYaw, FRotator::NormalizeAxis(EdgeYaw + 180.f) };

            for (float TargetYaw : CandidateYaws)
            {
                // 회전 차이도 스코어에 반영
                const float DeltaYaw = FMath::Abs(FMath::FindDeltaAngleDegrees(InOutRotation.Yaw, TargetYaw));

                // 가상 Transform: 위치는 현재 프리뷰 위치(스냅 적용 전), 회전은 TargetYaw
                const FTransform VirtualT(FRotator(0.f, TargetYaw, 0.f), InOutLocation);

                // 가상 프리뷰 엣지 월드 계산
                TArray<FBuildingEdge> PrevEdgesWorld;
                PreviewActor->GetEdgesWorldWithTransform(VirtualT, PrevEdgesWorld);

                for (const FBuildingEdge& EPw : PrevEdgesWorld)
                {
                    const FVector2D PrevDir2d = EPw.Dir2D();
                    if (PrevDir2d.IsNearlyZero())
                    {
                        continue;
                    }

                    // 각도 필터
                    if (FMath::Abs(FVector2D::DotProduct(PrevDir2d, OtherDir2d)) < EdgeParallelCosThreshold)
                    {
                        continue;
                    }

                    const float PrevLen = EPw.Length2D();
                    const float OtherLen = EO.Length2D();
                    if (PrevLen <= KINDA_SMALL_NUMBER || OtherLen <= KINDA_SMALL_NUMBER)
                    {
                        continue;
                    }

                    // 슬라이딩 최대(모서리 접촉 가능)
                    const float SlideHalfRange = 0.5f * (PrevLen + OtherLen);

                    // "붙이기 + 슬라이드": 프리뷰 엣지 중점을 타겟 엣지 연장선에 투영
                    const FVector2D PrevMid2d = FVector2D(EPw.Mid().X, EPw.Mid().Y);
                    const FVector2D Closest2d = ClosestPointOnExtendedLine2D(PrevMid2d, EO, OtherDir2d, SlideHalfRange);
                    const FVector2D DeltaBase2d = Closest2d - PrevMid2d;

                    // 타겟 법선(2D)
                    const FVector2D Normal2d(OtherDir2d.Y, OtherDir2d.X);

                    // "건물 크기만큼 띄우기" 후보: 0 / 프리뷰 / 타겟
                    const float PrevOffset = GetPerpFullSizeForEdge(PreviewActor, EPw);
                    const float OtherOffset = GetPerpFullSizeForEdge(OtherBuilding, EO);
                    const float Offsets[3] = { 0.f, PrevOffset, OtherOffset };

                    for (float Offset : Offsets)
                    {
                        // 1차 이동(붙이기+슬라이드+오프셋)
                        const FVector2D DeltaOff2d = DeltaBase2d + Normal2d * Offset;

                        // 키포인트 스냅(슬라이드 방향 성분만 추가)
                        TArray<FVector> PrevKeys;
                        SampleKeyPointsOnEdge(EPw, PrevKeys);
                        for (FVector& P : PrevKeys)
                        {
                            P.X += DeltaOff2d.X;
                            P.Y += DeltaOff2d.Y;
                        }

                        TArray<FVector> OtherKeys;
                        SampleKeyPointsOnEdge(EO, OtherKeys);

                        float BestAlong = 0.f;
                        float BestKeyDistSq = KeyPointSnapMaxDistance * KeyPointSnapMaxDistance;
                        bool bKeySnap = false;

                        for (const FVector& PK : PrevKeys)
                        {
                            const FVector2D PK2d(PK.X, PK.Y);
                            for (const FVector& OK : OtherKeys)
                            {
                                const FVector2D OK2d(OK.X, OK.Y);
                                const FVector2D Diff = OK2d - PK2d;

                                const float DistSq = Diff.SizeSquared();
                                if (DistSq > KeyPointSnapMaxDistance * KeyPointSnapMaxDistance)
                                {
                                    continue;
                                }

                                const float Along = FVector2D::DotProduct(Diff, OtherDir2d);

                                if (DistSq < BestKeyDistSq)
                                {
                                    BestKeyDistSq = DistSq;
                                    BestAlong = Along;
                                    bKeySnap = true;
                                }
                            }
                        }

                        FVector2D DeltaFinal2d = DeltaOff2d;

                        if (bKeySnap)
                        {
                            const FVector2D NewMid2d = PrevMid2d + DeltaOff2d + OtherDir2d * BestAlong;
                            const FVector2D ClampedMid2d = ClosestPointOnExtendedLine2D(NewMid2d, EO, OtherDir2d, SlideHalfRange);
                            DeltaFinal2d = ClampedMid2d - PrevMid2d;
                        }

                        const float DeltaZ = EO.Mid().Z - EPw.Mid().Z;

                        const FVector DeltaWorld(DeltaFinal2d.X, DeltaFinal2d.Y, DeltaZ);

                        // 최종 스코어: 이동량 + 회전량
                        const float MoveCost = DeltaWorld.SizeSquared();
                        const float RotCost = DeltaYaw * DeltaYaw;
                        const float TotalScore = MoveCost + RotCost;

                        if (TotalScore < BestScore && MoveCost <= SnapMaxDistance * SnapMaxDistance)
                        {
                            BestScore = TotalScore;
                            BestDelta = DeltaWorld;
                            BestYaw = TargetYaw;
                            bFound = true;
                        }
                    }
                }
            }
        }
    }

    if (!bFound)
    {
        return false;
    }

    // 적용
    InOutRotation.Yaw = FRotator::NormalizeAxis(BestYaw);
    InOutLocation += BestDelta;
    return true;
}

void UBuildManagerComponent::Server_TryPlace_Implementation(FName BuildingRow, const FVector& Location, const FRotator& Rotation)
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
        Spawned->DrawEdgesDebug(true, -1.f);
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

void UBuildManagerComponent::SampleKeyPointsOnEdge(const FBuildingEdge& E, TArray<FVector>& OutPts) const
{
    OutPts.Reset(5);
    OutPts.Add(FMath::Lerp(E.A, E.B, 0.f));
    OutPts.Add(FMath::Lerp(E.A, E.B, 0.25f));
    OutPts.Add(FMath::Lerp(E.A, E.B, 0.5f));
    OutPts.Add(FMath::Lerp(E.A, E.B, 0.75f));
    OutPts.Add(FMath::Lerp(E.A, E.B, 1.f));
}

FVector2D UBuildManagerComponent::ClosestPointOnExtendedLine2D(const FVector2D& Point2D, const FBuildingEdge& TargetEdgeWorld, const FVector2D& TargetDir2D, float HalfRange) const
{
    const FVector2D A2(TargetEdgeWorld.A.X, TargetEdgeWorld.A.Y);
    const FVector2D B2(TargetEdgeWorld.B.X, TargetEdgeWorld.B.Y);
    const FVector2D Mid = (A2 + B2) * 0.5f;

    const float S = FVector2D::DotProduct(Point2D - Mid, TargetDir2D);
    const float ClampedS = FMath::Clamp(S, -HalfRange, +HalfRange);

    return Mid + TargetDir2D * ClampedS;
}

float UBuildManagerComponent::GetPerpFullSizeForEdge(const ABaseBuilding* Building, const FBuildingEdge& EdgeWorld) const
{
    if (!Building) return 0.f;

    const FVector LocalDir = Building->GetActorTransform().InverseTransformVectorNoScale(EdgeWorld.B - EdgeWorld.A);

    const FVector AbsDir = LocalDir.GetAbs();

    if (AbsDir.X >= AbsDir.Y)
    {
        return 2.f * Building->GetBuildingBoxExtent().Y;
    }

    return 2.f * Building->GetBuildingBoxExtent().X;
}

