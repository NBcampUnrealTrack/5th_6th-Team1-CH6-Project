#pragma once

#include "CoreMinimal.h"
#include "Building/RangedTurret.h"
#include "MissileTurret.generated.h"

class UStaticMesh;

UCLASS()
class BULLETANT_API AMissileTurret : public ARangedTurret
{
	GENERATED_BODY()

public:
	AMissileTurret();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void UpdateAim(float DeltaSeconds) override;

	virtual void ApplyPreviewMode() override;

protected:
	virtual void ExecuteAttack() override;
	virtual float GetAttackInterval() const override;

private:
	void StartFireSequence();
	void FireSequenceStep();

	void StartReloadSequence();
	void ReloadSequenceStep();

	bool PrepareLaunchSolution();
	void UpdateMissileAim(float DeltaSeconds);

	bool RefreshLaunchSolution();
	void StartLaunchSolutionTimer();
	void StopLaunchSolutionTimer();
	void RefreshLaunchSolutionTick();

	bool IsCurrentTargetStillValid() const;

	float GetCycleDuration() const;
	float GetFireStepInterval() const;
	float GetReloadStepInterval() const;

	void BuildMissileVisuals();
	void SetMissileVisualLoaded(int32 Index, bool bLoaded);

private:
	UPROPERTY(EditDefaultsOnly, Category = "Missile|Timing", meta = (ClampMin = "0.1", ClampMax = "0.9"))
	float FirePhaseRatio = 0.5f;

	UPROPERTY(EditDefaultsOnly, Category = "Missile|Visual")
	TObjectPtr<UStaticMesh> MissileVisualMesh;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UStaticMeshComponent>> MissileVisuals;

	UPROPERTY(Transient)
	TArray<bool> bMissileLoaded;

	UPROPERTY(Transient)
	int32 SequenceIndex = 0;

	UPROPERTY(Transient)
	bool bCycleInProgress = false;

	UPROPERTY(Transient)
	bool bHasValidLaunchSolution = false;

	UPROPERTY(Transient)
	FVector CachedLaunchDirection = FVector::ForwardVector;

	UPROPERTY(Transient)
	FVector CachedImpactPoint = FVector::ZeroVector;

	FTimerHandle SequenceTimerHandle;

	UPROPERTY(Transient)
	TWeakObjectPtr<AActor> CachedSolutionTarget;

	UPROPERTY(Transient)
	FVector CachedSolutionTargetLocation = FVector::ZeroVector;

	FTimerHandle LaunchSolutionTimerHandle;
};