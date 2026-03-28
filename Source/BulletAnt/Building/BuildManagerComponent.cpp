
#include "Building/BuildManagerComponent.h"
#include "Building/BaseBuilding.h"
#include "Engine/OverlapResult.h"
#include "UI/UISubsystem.h"
#include "UI/UW_BuildMenu.h"
#include <InputActionValue.h>
#include "Framework/BAGameMode.h" 
#include "Components/PrimitiveComponent.h"
#include "GameFramework/Pawn.h"
#include <Kismet/GameplayStatics.h>
#include "AbilitySystemComponent.h"
#include "GAS/BAGameplayTags.h"

UBuildManagerComponent::UBuildManagerComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    SetComponentTickEnabled(false);
    SetIsReplicatedByDefault(true);
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
    bSnapMode = true;
    RefreshCachedRef();
    RefreshCategoryCache();
    CurrentYaw = 0.f;
    SetCurrentBuildingRow(DefaultBuildingRow);
    SelectCategory(CurrentCategory);
    SetComponentTickEnabled(true);    
}

void UBuildManagerComponent::ExitBuildMode()
{
    bBuildMenuOpen = true;
    ToggleBuildMenu();
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

void UBuildManagerComponent::ToggleBuildMenu()
{
    if (!bBuildMode)
    {
        return;
    }

    APlayerController* PC = CachedPC.Get();
    if (!PC)
    {
        RefreshCachedRef();
        PC = CachedPC.Get();
    }

    if (!PC)
    {
        return;
    }

    ULocalPlayer* LP = PC->GetLocalPlayer();
    if (!LP)
    {
        return;
    }

    UUISubsystem* UIS = LP->GetSubsystem<UUISubsystem>();
    if (!UIS)
    {
        return;
    }

    if (bBuildMenuOpen)
    {
        UIS->HideUI(EUIType::BuildMenu);
        UIS->ApplyGameOnlyInputMode();
        bBuildMenuOpen = false;
        return;
    }

    UUW_BuildMenu* BuildMenuWidget = UIS->ShowUI<UUW_BuildMenu>(EUIType::BuildMenu);
    if (!IsValid(BuildMenuWidget))
    {
        return;
    }

    BuildMenuWidget->OnBuildMenuSelected.RemoveDynamic(this, &UBuildManagerComponent::OnBuildMenuSelected);
    BuildMenuWidget->OnBuildMenuSelected.AddDynamic(this, &UBuildManagerComponent::OnBuildMenuSelected);

    UIS->ApplyGameAndUIInputMode(BuildMenuWidget);
    bBuildMenuOpen = true;
}

void UBuildManagerComponent::RequestDemolish(ABaseBuilding* TargetBuilding)
{
    if (!IsValid(TargetBuilding))
    {
        return;
    }

    AActor* OwnerActor = GetOwner();
    if (!OwnerActor)
    {
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("[BuildManager] RequestDemolish OwnerAuthority=%d Target=%s"),
        OwnerActor->HasAuthority() ? 1 : 0,
        *TargetBuilding->GetName());

    if (OwnerActor->HasAuthority())
    {
        TargetBuilding->RequestDemolish(OwnerActor);
    }
    else
    {
        Server_RequestDemolish(TargetBuilding);
    }
}

void UBuildManagerComponent::RequestRepair(ABaseBuilding* TargetBuilding)
{
    if (!IsValid(TargetBuilding))
    {
        return;
    }

    AActor* OwnerActor = GetOwner();
    if (!OwnerActor)
    {
        return;
    }

    if (OwnerActor->HasAuthority())
    {
        Server_RequestRepair_Implementation(TargetBuilding);
    }
    else
    {
        Server_RequestRepair(TargetBuilding);
    }
}

void UBuildManagerComponent::Server_RequestRepair_Implementation(ABaseBuilding* TargetBuilding)
{
    if (!IsValid(TargetBuilding))
    {
        return;
    }

    AActor* OwnerActor = GetOwner();
    APawn* OwnerPawn = Cast<APawn>(OwnerActor);
    if (!OwnerPawn)
    {
        return;
    }

    //const float MaxUseDist = 500.f;
    //if (FVector::DistSquared(OwnerPawn->GetActorLocation(), TargetBuilding->GetActorLocation()) > FMath::Square(MaxUseDist))
    //{
    //    return;
    //}

    if (!TargetBuilding->CanRepair())
    {
        return;
    }

    const float MaxHeal = TargetBuilding->GetRepairHealAmount();
    const float MissingHealth = TargetBuilding->GetMaxHealth() - TargetBuilding->GetCurrentHealth();
    const float ActualHeal = FMath::Min(MaxHeal, MissingHealth);

    if (ActualHeal <= KINDA_SMALL_NUMBER)
    {
        return;
    }

    const float HealRatio = (MaxHeal > 0.f) ? (ActualHeal / MaxHeal) : 0.f;

    TMap<EOreType, int32> CostMap = TargetBuilding->GetRepairCost();

    for (TPair<EOreType, int32>& Pair : CostMap)
    {
        Pair.Value = FMath::CeilToInt(Pair.Value * HealRatio);
    }

    ABAGameMode* GM = GetWorld() ? GetWorld()->GetAuthGameMode<ABAGameMode>() : nullptr;
    if (!GM)
    {
        return;
    }

    if (!GM->TrySpendOre(CostMap))
    {
        return;
    }

    TargetBuilding->Repair(ActualHeal);
}

void UBuildManagerComponent::Server_RequestDemolish_Implementation(ABaseBuilding* TargetBuilding)
{
    if (!IsValid(TargetBuilding))
    {
        return;
    }

    AActor* OwnerActor = GetOwner();
    APawn* OwnerPawn = Cast<APawn>(OwnerActor);
    if (!OwnerPawn)
    {
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("[BuildManager] Server_RequestDemolish_Implementation Target=%s"),
        *TargetBuilding->GetName());

    const float MaxUseDist = 500.f;
    if (FVector::DistSquared(OwnerPawn->GetActorLocation(), TargetBuilding->GetActorLocation()) > FMath::Square(MaxUseDist))
    {
        UE_LOG(LogTemp, Warning, TEXT("[BuildManager] Demolish rejected: too far"));
        return;
    }

    TargetBuilding->RequestDemolish(OwnerActor);
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

    FTransform SpawnTransform(FRotator::ZeroRotator, FVector::ZeroVector);

    FActorSpawnParameters Params;
    Params.Owner = GetOwner();
    Params.Instigator = Cast<APawn>(GetOwner());
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    PreviewActor = World->SpawnActorDeferred<ABaseBuilding>(
        BuildingClass,
        SpawnTransform,
        Params.Owner,
        nullptr,
        Params.SpawnCollisionHandlingOverride
    );

    if (!PreviewActor) 
    {
        return;
    }

    if (CachedBuildingRow)
    {
        PreviewActor->ApplyBuildingRow(*CachedBuildingRow);
    }

    PreviewActor->SetReplicates(false);
    PreviewActor->SetPreviewMode(true);

    UGameplayStatics::FinishSpawningActor(PreviewActor, SpawnTransform);
    PreviewActor->ApplyPreviewMode();
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

bool UBuildManagerComponent::TrySnapPreview(FVector & InOutLocation, FRotator & InOutRotation)
{
    UWorld* World = GetWorld();
    if (!World || !PreviewActor)
    {
        return false;
    }

    FCollisionQueryParams Params(SCENE_QUERY_STAT(SnapSearch), false);
    Params.AddIgnoredActor(PreviewActor);

    FCollisionObjectQueryParams Obj;
    Obj.AddObjectTypesToQuery(ECC_GameTraceChannel1);

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

    TArray<FBuildingEdge> PrevLocalEdges;
    PreviewActor->GetEdgesLocal(PrevLocalEdges);

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

            const float OtherYaw = FMath::RadiansToDegrees(FMath::Atan2(OtherDir2d.Y, OtherDir2d.X));

            for (const FBuildingEdge& EPLocal : PrevLocalEdges)
            {
                const FVector2D PrevLocalDir2d = EPLocal.Dir2D();
                if (PrevLocalDir2d.IsNearlyZero())
                {
                    continue;
                }

                const float PrevLocalYaw = FMath::RadiansToDegrees(
                    FMath::Atan2(PrevLocalDir2d.Y, PrevLocalDir2d.X)
                );

                const float CandidateYaws[2] =
                {
                    FRotator::NormalizeAxis(OtherYaw - PrevLocalYaw),
                    FRotator::NormalizeAxis(OtherYaw + 180.f - PrevLocalYaw)
                };

                for (float TargetYaw : CandidateYaws)
                {
                    const float DeltaYaw = FMath::Abs(
                        FMath::FindDeltaAngleDegrees(InOutRotation.Yaw, TargetYaw)
                    );

                    FTransform VirtualT = PreviewActor->GetActorTransform();
                    VirtualT.SetLocation(InOutLocation);
                    VirtualT.SetRotation(FQuat(FRotator(0.f, TargetYaw, 0.f)));

                    TArray<FBuildingEdge> PrevEdgesWorld;
                    PreviewActor->GetEdgesWorldWithTransform(VirtualT, PrevEdgesWorld);

                    for (const FBuildingEdge& EPw : PrevEdgesWorld)
                    {
                        const FVector2D PrevDir2d = EPw.Dir2D();
                        if (PrevDir2d.IsNearlyZero())
                        {
                            continue;
                        }

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

                        const float SlideHalfRange = 0.5f * (PrevLen + OtherLen);

                        const FVector2D PrevMid2d(EPw.Mid().X, EPw.Mid().Y);
                        const FVector2D Closest2d = ClosestPointOnExtendedLine2D(PrevMid2d, EO, OtherDir2d, SlideHalfRange);
                        const FVector2D DeltaOff2d = Closest2d - PrevMid2d;

                        const float DeltaZ = EO.Mid().Z - EPw.Mid().Z;
                        const FVector DeltaWorld(DeltaOff2d.X, DeltaOff2d.Y, DeltaZ);

                        const float MoveCost = DeltaWorld.SizeSquared();

                        // 회전 튐 방지: 회전 비용 가중치 올리기
                        const float RotCost = DeltaYaw * DeltaYaw * 4.f;
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

    FTransform SpawnTransform(Rotation, Location);

    FActorSpawnParameters Params;
    Params.Owner = GetOwner();
    Params.Instigator = Cast<APawn>(GetOwner());
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    ABaseBuilding* Spawned = World->SpawnActorDeferred<ABaseBuilding>(
        Row->BuildingClass,
        SpawnTransform,
        Params.Owner,
        Params.Instigator,
        Params.SpawnCollisionHandlingOverride
    );

    if (!Spawned)
    {
        return;
    }

    Spawned->ApplyBuildingRow(*Row);

    UGameplayStatics::FinishSpawningActor(Spawned, SpawnTransform);

    float Coverage = 0.f;
    TSet<TWeakObjectPtr<ABaseBuilding>> Supporters;
    const bool bHasCoverageResult = Spawned->ComputeSupportCoverage(
        Spawned->GetActorTransform(),
        Coverage,
        Supporters
    );

    if (!bHasCoverageResult || Coverage < Spawned->MinSupportCoverage)
    {
        Spawned->OnDeath();
        return;
    }

    Spawned->Server_RegisterSupports(Supporters);

    if (UAbilitySystemComponent* BuildingASC = Spawned->GetAbilitySystemComponent())
    {
        FGameplayCueParameters CueParams;
        CueParams.Location = Spawned->GetActorLocation();
        CueParams.Instigator = GetOwner();
        CueParams.EffectCauser = Spawned;
        CueParams.EffectContext = BuildingASC->MakeEffectContext();

        BuildingASC->ExecuteGameplayCue(TAG_GameplayCue_Building_Placed, CueParams);
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

    float Coverage = 0.f;
    TSet<TWeakObjectPtr<ABaseBuilding>> Dummy;
    const bool bOk = PreviewActor->ComputeSupportCoverage(PreviewActor->GetActorTransform(), Coverage, Dummy);

    if (!bOk || Coverage < PreviewActor->MinSupportCoverage)
    {
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
