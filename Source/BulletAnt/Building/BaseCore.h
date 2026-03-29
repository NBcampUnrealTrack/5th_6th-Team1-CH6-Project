// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Building/BaseBuilding.h"
#include "AbilitySystemComponent.h"
#include "BaseCore.generated.h"

class UMaterialInterface;
class UMaterialInstanceDynamic;
class UGameplayEffect;

UCLASS()
class BULLETANT_API ABaseCore : public ABaseBuilding
{
	GENERATED_BODY()
	
public:
	virtual void Use_Implementation(AActor* User) override;

	const TArray<FVector>& GetAnchors() const;
	
protected:
	ABaseCore();

	virtual void BeginPlay() override;
	virtual void OnDeath() override;

	UFUNCTION(NetMulticast, Reliable)
	void Multi_ShowResult();

	void FindAnchors();
	void InitializeCoreMaterial();
	void UpdateCoreMaterialHealthRatio();

	void HandleHealthChanged(const FOnAttributeChangeData& ChangeData);
	void HandleRegen();

	UPROPERTY(EditDefaultsOnly)
	int32 ScanCount = 72;

	UPROPERTY(VisibleAnywhere)
	TArray<FVector> Anchors;

	UPROPERTY(EditDefaultsOnly, Category = "Core|Material")
	TObjectPtr<UMaterialInterface> CoreMaterial;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> CoreMID;

	FDelegateHandle HealthChangedDelegateHandle;

	FTimerHandle RegenTimerHandle;

	UPROPERTY(EditDefaultsOnly, Category = "Regen")
	float RegenPercentPerSecond = 0.0005f; // 0.05%

	UPROPERTY(EditDefaultsOnly, Category = "Core|Regen")
	TSubclassOf<UGameplayEffect> RegenHealEffect;
};
