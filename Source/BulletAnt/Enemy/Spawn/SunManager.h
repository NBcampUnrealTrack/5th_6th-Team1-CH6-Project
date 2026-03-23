// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SunManager.generated.h"

class ADirectionalLight;
class ABAGameState;

UCLASS()
class BULLETANT_API ASunManager : public AActor
{
	GENERATED_BODY()

protected:
	ASunManager();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	void TryCachingGameState();

	UFUNCTION(NetMulticast, Reliable)
	void SetSunInitRotator(int32 InInitWaveTime);

	void RotateSun();

	void OnInitWaveTime(int32 InInitWaveTime);

	void OnWaveTime();

protected:
	UPROPERTY(EditAnywhere, Category = "Light")
	TObjectPtr<ADirectionalLight> Sun;

	UPROPERTY(EditAnywhere, Category = "Light")
	TObjectPtr<ADirectionalLight> Moon;

	TObjectPtr<ABAGameState> CachedGameState;

	float RotationPerMSec = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Lighting")
	float SunTiltAngle = 60.0f;
	
	FRotator SunRise;

	FVector CustomAxis;




	FTimerHandle CachingGameStateTimerHandle;
	FTimerHandle RotatingLightTimerHandle;
};
