
#include "Building/BuildManagerComponent.h"
#include "Building/BaseBuilding.h"
#include "Engine/OverlapResult.h"
#include "UI/UISubsystem.h"
#include "UI/UW_BuildMenu.h"
#include <InputActionValue.h>
#include "Framework/BAGameMode.h" 
#include "Components/PrimitiveComponent.h"

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
    PreviewActor->SetActorLocationAndRotation(FreeLocation, Rotation);
    FVector FinalLocation = FreeLocation;
    if (bSnapMode)
    {
        TrySnapPreview(FinalLocation, Rotation);
    }

    PreviewActor->SetActorLocationAndRotation(FinalLocation, Rotation);
    PreviewActor->DrawEdgesDebug(false, 0.02f);

    // 설치 가능 판정
    if (bHasValidSurface)
    {
        bCanPlace = CheckCanPlaceAt();
    }
    else
    {
        bCanPlace = false;
    }
    PreviewActor->SetCanPlace(bCanPlace);
}

void UBuildManagerComponent::EnterBuildMode()
{
    if (bBuildMode || !BuildingTable)
    {
        return;
    }

    bBuildMode = true;
    RefreshCachedRef();
    RefreshCategoryCache();
    CurrentYaw = 0.f;
    SetCurrentBuildingRow(DefaultBuildingRow);
    SelectCategory(CurrentCategory);
    SetComponentTickEnabled(true);

    //if (auto* PC = CachedPC.Get())
    //{
    //    if (auto* LP = PC->GetLocalPlayer())
    //    {
    //        if (auto* UIS = LP->GetSubsystem<UUISubsystem>())
    //        {
    //            UUW_BuildMenu* BuildMenuWidget = UIS->ShowUI<UUW_BuildMenu>(EUIType::BuildMenu);
    //            if (IsValid(BuildMenuWidget))
    //            {
    //                BuildMenuWidget->OnBuildMenuSelected.RemoveDynamic(this, &UBuildManagerComponent::OnBuildMenuSelected);
    //                BuildMenuWidget->OnBuildMenuSelected.AddDynamic(this, &UBuildManagerComponent::OnBuildMenuSelected);
    //                PC->SetShowMouseCursor(true);
    //            }
    //        }
    //    }
    //}
    
}

void UBuildManagerComponent::ExitBuildMode()
{
    bBuildMode = false;

    //if (auto* PC = CachedPC.Get())
    //{
    //    if (auto* LP = PC->GetLocalPlayer())
    //    {
    //        if (auto* UIS = LP->GetSubsystem<UUISubsystem>())
    //        {
    //           UIS->HideUI(EUIType::BuildMenu);
    //           PC->SetShowMouseCursor(false);
    //        }
    //    }
    //}

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

void UBuildManagerComponent::OnBuildMenuSelected(FName NewRow)
{
    SetCurrentBuildingRow(NewRow);
}

void UBuildManagerComponent::TryPlace()
{
    if (!bBuildMode)
    {
        return;
    }

    if (!PreviewActor || !bCanPlace)
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

void UBuildManagerComponent::SpawnPreview(TSubclassOf<ABaseBuilding> BuildingClass)
{
    UWorld* World = GetWorld();
    if (!World || !*BuildingClass) 
    {
        return;
    }

    if (PreviewActor)
    {
        PreviewActor->Destroy();
        PreviewActor = nullptr;
    }

    FActorSpawnParameters Params;
    Params.Owner = GetOwner();
    Params.Instigator = Cast<APawn>(GetOwner());
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    PreviewActor = World->SpawnActor<ABaseBuilding>(BuildingClass, FVector::ZeroVector, FRotator::ZeroRotator, Params);
    if (!PreviewActor) 
    {
        return;
    }

    PreviewActor->SetReplicates(false);
    PreviewActor->SetPreviewMode(true);
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

    // 주변 빌딩 후보 찾기
    FCollisionQueryParams Params(SCENE_QUERY_STAT(SnapSearch), false);
    Params.AddIgnoredActor(PreviewActor);

    FCollisionObjectQueryParams Obj;
    Obj.AddObjectTypesToQuery(ECC_GameTraceChannel1); // Building

    TArray<FOverlapResult> Overlaps;
    const bool bAnyOverlap = World->OverlapMultiByObjectType(
        Overlaps,
        InOutLocation,
        FQuat::Identity,
        Obj,
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
                    const FVector2D DeltaOff2d = Closest2d - PrevMid2d;

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

    if (ABAGameMode* GM = GetWorld() ? GetWorld()->GetAuthGameMode<ABAGameMode>() : nullptr)
    {
        if (!GM->TrySpendOre(Row->BuildCost))
        {
            return;
        }
    }
    else
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
        Spawned->DrawEdgesDebug(true, -1.f);
    }
}

bool UBuildManagerComponent::CheckCanPlaceAt() const
{
    if (!PreviewActor)
    {
        return false;
    }

    TArray<UPrimitiveComponent*> Prims;
    PreviewActor->GetPlacementPrimitives(Prims);
    
    if (Prims.Num() == 0)
    {
        return false;
    }

    for (UPrimitiveComponent* Prim : Prims)
    {
        if (!Prim) 
        {
            continue;
        }

        TArray<UPrimitiveComponent*> Overlapping;
        Prim->GetOverlappingComponents(Overlapping);

        for (UPrimitiveComponent* OtherComp : Overlapping)
        {
            if (!OtherComp)
            {
                continue;
            }

            AActor* OtherActor = OtherComp->GetOwner();
            if (!OtherActor || OtherActor == PreviewActor)
            {
                continue;
            }

            if (OtherComp->GetCollisionObjectType() != ECC_GameTraceChannel1 && OtherComp->GetCollisionObjectType() != ECC_Pawn)
            {
                continue;
            }

            if (OtherActor->ActorHasTag(GroundActorTag))
            {
                continue;
            }

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

void UBuildManagerComponent::RefreshCategoryCache()
{
    CategoryRows.Reset();
    if (!BuildingTable)
    {
        return;
    }

    const TArray<FName> RowNames = BuildingTable->GetRowNames();

    for (const FName& RowName : RowNames)
    {
        const FBuildingRow* Row = BuildingTable->FindRow<FBuildingRow>(RowName, TEXT("RefreshCategoryCache"));
        if (!Row || !Row->BuildingClass)
        {
            continue;
        }

        CategoryRows.FindOrAdd(Row->Category).Add(RowName);
    }

    for (auto& Pair : CategoryRows)
    {
        Pair.Value.Sort([this](const FName& A, const FName& B)
            {
                const FBuildingRow* RA = BuildingTable->FindRow<FBuildingRow>(A, TEXT("SortA"));
                const FBuildingRow* RB = BuildingTable->FindRow<FBuildingRow>(B, TEXT("SortB"));

                const int32 OA = RA ? RA->Order : 0;
                const int32 OB = RB ? RB->Order : 0;

                if (OA != OB) 
                {
                    return OA < OB;
                }
                return A.LexicalLess(B);
            });
    }

    const TArray<FName>* CurList = CategoryRows.Find(CurrentCategory);
    if (!CurList || CurList->Num() == 0)
    {
        for (const auto& Any : CategoryRows)
        {
            if (Any.Value.Num() > 0)
            {
                CurrentCategory = Any.Key;
                CurrentIndexInCategory = 0;
                break;
            }
        }
        return;
    }

    CurrentIndexInCategory = FMath::Clamp(CurrentIndexInCategory, 0, CurList->Num() - 1);
}

void UBuildManagerComponent::SetCurrentBuildingRow(FName NewRow)
{
    if (!BuildingTable) 
    {
        return;
    }

    const FBuildingRow* Row = BuildingTable->FindRow<FBuildingRow>(NewRow, TEXT("SetCurrentBuildingRow"));
    if (!Row || !Row->BuildingClass) 
    {
        return;
    }

    CurrentBuildingRow = NewRow;
    CachedBuildingRow = Row;

    SpawnPreview(Row->BuildingClass);
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


void UBuildManagerComponent::SelectCategory(EBuildCategory NewCategory)
{
    CurrentCategory = NewCategory;
    CurrentIndexInCategory = 0;

    const TArray<FName>* List = CategoryRows.Find(CurrentCategory);
    if (!List || List->Num() == 0)
    {
        return;
    }

    SetCurrentBuildingRow((*List)[CurrentIndexInCategory]);
}

void UBuildManagerComponent::CycleInCategory(int32 Delta)
{
    const TArray<FName>* List = CategoryRows.Find(CurrentCategory);
    if (!List || List->Num() == 0)
    {
        return;
    }

    const int32 N = List->Num();
    CurrentIndexInCategory = (CurrentIndexInCategory + Delta) % N;
    if (CurrentIndexInCategory < 0)
    {
        CurrentIndexInCategory += N;
    }

    SetCurrentBuildingRow((*List)[CurrentIndexInCategory]);
}
