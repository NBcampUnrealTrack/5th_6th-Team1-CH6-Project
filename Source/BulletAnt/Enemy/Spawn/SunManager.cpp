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
    float TotalWaveTime = CachedGameState->GetInitWavePreparationTime() + 100.f;
    RotationPerMSec = 360 * 0.1f / TotalWaveTime;
    float NightAngle = 360.f * 100.f / TotalWaveTime;

    // 1. 극단값 방어 (Clamp로 안전 범위 설정)
    // Alpha는 0도 ~ 179.9도 사이로 제한
    float SafeAlpha = FMath::Clamp(NightAngle, 0.001f, 179.9f);
    // Theta는 0도 ~ 179.9도 사이로 제한 (sin가 0이 되지 않도록)
    float SafeTheta = FMath::Clamp(SunTiltAngle, 0.f, 179.f);

    // 2. Degree -> Radian 변환
    float AlphaRad = FMath::DegreesToRadians(SafeAlpha);
    float ThetaRad = FMath::DegreesToRadians(SafeTheta);

    // 3. 공식 적용: w = 2 * atan( tan(alpha/2) * cos(theta) )
    float TanPart = FMath::Tan(AlphaRad / 2.0f);
    float SinPart = FMath::Sin(ThetaRad);

    // Atan은 라디안 값을 반환합니다.
    float W_Rad = FMath::Atan(TanPart * SinPart);

    // 4. 결과값 Radian -> Degree 변환 (에디터나 Rotator에 넣기 위함)
    float W_Deg = FMath::RadiansToDegrees(W_Rad);

    SunRise = FRotator(0, 90 - W_Deg, 0);
    Sun->SetActorRotation(SunRise);

    // 2. 회전축 계산: -X축을 Y축(Up/Right는 기준에 따라 다름, 여기선 월드 Y) 기준으로 60도 회전
    // 언리얼 좌표계: X(Forward), Y(Right), Z(Up)
    FVector AxisY = FVector::RightVector; // (-1, 0, 0)
    FVector RotationAxisX = FVector::ForwardVector; // (0, 1, 0)

    // -X축을 Y축 중심으로 60도 회전시켜 커스텀 축(Axis) 생성
    CustomAxis = AxisY.RotateAngleAxis(-SunTiltAngle, RotationAxisX);
}

void ASunManager::RotateSun()
{
    // 1. 기존 Rotator를 쿼터니언으로 변환
    FRotator BaseRotator(Sun->GetActorRotation());
    FQuat QBase = BaseRotator.Quaternion();

    // 3. 이 CustomAxis를 기준으로 10도 회전하는 쿼터니언 생성
    FQuat QDelta = FQuat(CustomAxis, FMath::DegreesToRadians(RotationPerMSec));

    // 4. 기존 쿼터니언에 회전 적용 (쿼터니언 곱셈 순서에 주의: QDelta * QBase는 Local 기준 회전)
    FQuat QResult = QDelta * QBase;
    Sun->SetActorRotation(QResult);

    float MoonIntensity = FMath::Clamp(0.1f + Sun->GetActorRotation().Pitch * 0.01f, 0.f, 0.1f);
    Moon->GetLightComponent()->SetIntensity(MoonIntensity);
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

    // 3. 이 CustomAxis를 기준으로 10도 회전하는 쿼터니언 생성
    FQuat QDelta = FQuat(CustomAxis, FMath::DegreesToRadians(Angle));

    // 4. 기존 쿼터니언에 회전 적용 (쿼터니언 곱셈 순서에 주의: QDelta * QBase는 Local 기준 회전)
    FQuat QResult = QDelta * QBase;
    Sun->SetActorRotation(QResult);
}
