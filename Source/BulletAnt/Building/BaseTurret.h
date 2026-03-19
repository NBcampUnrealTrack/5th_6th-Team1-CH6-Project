
#pragma once

#include "CoreMinimal.h"
#include "Building/BaseBuilding.h"
#include "BaseTurret.generated.h"

class UTurretDataAsset;
class USphereComponent;

UCLASS()
class BULLETANT_API ABaseTurret : public ABaseBuilding
{
	GENERATED_BODY()
	
public:
	ABaseTurret();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaSeconds) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void SetPreviewMode(bool bInPreview) override;

protected:
	// IOnDeathInterface
	virtual void OnDeath() override;
	virtual void OnRep_Dead() override;

	virtual bool CanStartAttack() const;
	virtual float GetAttackInterval() const;
	virtual void ExecuteAttack();

	virtual void ApplyTurretData();
	virtual void UpdateAim(float DeltaSeconds);
	virtual AActor* SelectBestTarget() const;

	void StartFireLoop();
	void StopFireLoop();
	void HandleAttackTick();

private:
	UFUNCTION()
	void OnTargetBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
	UFUNCTION()
	void OnTargetEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION()
	void UpdateCurrentTarget();

protected:
	UPROPERTY(VisibleAnywhere, Category = "Turret|Mesh")
	TObjectPtr<UStaticMeshComponent> BodyMesh;

	UPROPERTY(VisibleAnywhere, Category = "Turret|Mesh")
	TObjectPtr<UStaticMeshComponent> BarrelMesh;

	UPROPERTY(EditDefaultsOnly, Category = "Turret|Data")
	TObjectPtr<UTurretDataAsset> TurretData;

	UPROPERTY(VisibleAnywhere, Category = "Turret|Targeting")
	TObjectPtr<USphereComponent> TargetSearchingSphere;

	UPROPERTY()
	TArray<TWeakObjectPtr<AActor>> TargetCandidates;

	UPROPERTY(Replicated)
	TObjectPtr<AActor> CurrentTarget;

	FTimerHandle FireLoopTimerHandle;
	FTimerHandle TargetSearchTimer;
};
