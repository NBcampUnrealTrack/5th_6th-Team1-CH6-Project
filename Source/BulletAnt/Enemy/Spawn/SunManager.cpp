// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Spawn/SunManager.h"
#include "FrameWork/BAGameState.h"
#include "Engine/DirectionalLight.h"
#include "Enemy/Spawn/SpawnManagerSubsystem.h"
#include "Components/LightComponent.h"

ASunManager::ASunManager()
{
    bReplicates = true;
}

void ASunManager::BeginPlay()
{
	Super::BeginPlay();

    if (HasAuthority())
    {
        UWorld* World = GetWorld();
        if (!IsValid(World))
        {
            return;
        }
        USpawnManagerSubsystem* SpawnManager = World->GetSubsystem<USpawnManagerSubsystem>();
        if (!IsValid(SpawnManager))
        {
            return;
        }
        SpawnManager->OnInitWaveTimeChanged.AddUObject(this, &ASunManager::OnInitWaveTime);
    }
    
    GetWorldTimerManager().SetTimer(
        CachingGameStateTimerHandle,
        this,
        &ASunManager::TryCachingGameState,
        0.1f,
        true,
        0.f
    );
}

void ASunManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    GetWorldTimerManager().ClearAllTimersForObject(this);

    Super::EndPlay(EndPlayReason);
}

void ASunManager::TryCachingGameState()
{
    CachedGameState = GetWorld()->GetGameState<ABAGameState>();

    if (IsValid(CachedGameState))
    {
        GetWorldTimerManager().ClearTimer(CachingGameStateTimerHandle);

        SetSunInitRotator(CachedGameState->GetInitWavePreparationTime());

        CachedGameState->OnWaveTimeChanged.AddUObject(this, &ASunManager::OnWaveTime);

        GetWorldTimerManager().SetTimer(
            RotatingLightTimerHandle,
            this,
            &ASunManager::RotateSun,
            0.1f,
            true
        );
    }
}

void ASunManager::SetSunInitRotator_Implementation(int32 InInitWaveTime)
{
    float TotalWaveTime = CachedGameState->GetInitWavePreparationTime();
    RotationPerMSec = 360 * 0.1f / TotalWaveTime;
    float NightAngle = 360.f * CachedGameState->GetSpawnTime() / TotalWaveTime;

    float SafeAlpha = FMath::Clamp(NightAngle, 0.001f, 179.9f);
    float SafeTheta = FMath::Clamp(SunTiltAngle, 0.001f, 179.f);

    float AlphaRad = FMath::DegreesToRadians(SafeAlpha);
    float ThetaRad = FMath::DegreesToRadians(SafeTheta);

    float TanPart = FMath::Tan(AlphaRad / 2.0f);
    float SinPart = FMath::Sin(ThetaRad);

    float W_Rad = FMath::Atan(TanPart * SinPart);
    float W_Deg = FMath::RadiansToDegrees(W_Rad);

    SunRise = FRotator(0, 90 - W_Deg, 0);
    Sun->SetActorRotation(SunRise);

    FVector AxisY = FVector::RightVector;
    FVector RotationAxisX = FVector::ForwardVector;
    CustomAxis = AxisY.RotateAngleAxis(-SunTiltAngle, RotationAxisX);
}

void ASunManager::RotateSun()
{
    FRotator SunRotator = Sun->GetActorRotation();
    FRotator BaseRotator(SunRotator);
    FQuat QBase = BaseRotator.Quaternion();

    FQuat QDelta = FQuat(CustomAxis, FMath::DegreesToRadians(RotationPerMSec));

    FQuat QResult = QDelta * QBase;
    Sun->SetActorRotation(QResult);

    float MoonIntensity = FMath::Clamp(0.1f + SunRotator.Pitch * 0.01f, 0.f, 0.1f);
    Moon->GetLightComponent()->SetIntensity(MoonIntensity);

    if (SunRotator.Pitch > 5)
    {
        Sun->GetLightComponent()->Deactivate();
        Sun->SetActorHiddenInGame(true);
    }
    else
    {
        Sun->GetLightComponent()->Activate();
        Sun->SetActorHiddenInGame(false);
    }

    if (MoonIntensity > 0)
    {
        Moon->GetLightComponent()->Activate();
        Moon->SetActorHiddenInGame(false);
    }
    else
    {
        Moon->GetLightComponent()->Deactivate();
        Moon->SetActorHiddenInGame(true);
    }
}

void ASunManager::OnInitWaveTime(int32 InInitWaveTime)
{
    SetSunInitRotator(InInitWaveTime);
}

void ASunManager::OnWaveTime()
{
    float RotatingTime = CachedGameState->GetInitWavePreparationTime() - CachedGameState->GetWavePreparationTime();
    float Angle = RotatingTime * RotationPerMSec * 10;

    FRotator BaseRotator(SunRise);
    FQuat QBase = BaseRotator.Quaternion();

    FQuat QDelta = FQuat(CustomAxis, FMath::DegreesToRadians(Angle));
    FQuat QResult = QDelta * QBase;
    Sun->SetActorRotation(QResult);
}
