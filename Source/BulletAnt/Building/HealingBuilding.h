#pragma once

#include "CoreMinimal.h"
#include "Building/BaseBuilding.h"
#include "HealingBuilding.generated.h"

class UCapsuleComponent;
class ABACharacter;
class UGameplayEffect;

UCLASS()
class BULLETANT_API AHealingBuilding : public ABaseBuilding
{
	GENERATED_BODY()

public:
	AHealingBuilding();

protected:
	virtual void BeginPlay() override;
	virtual void OnRep_Dead() override;

protected:
	UFUNCTION()
	void OnHealZoneBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	UFUNCTION()
	void OnHealZoneEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex
	);

	void TickHealPlayers();
	bool CanHealCharacter(ABACharacter* Character) const;
	void HealCharacter(ABACharacter* Character);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Heal")
	TObjectPtr<UCapsuleComponent> HealZone;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Heal")
	float HealPercentPerSecond = 0.2f; // 초당 최대체력 20%

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Heal")
	float HealTickInterval = 1.0f;

	UPROPERTY()
	TSet<TWeakObjectPtr<ABACharacter>> OverlappingPlayers;

	FTimerHandle HealTimerHandle;

	UPROPERTY(EditDefaultsOnly, Category = "Heal")
	TSubclassOf<UGameplayEffect> HealEffect;
};