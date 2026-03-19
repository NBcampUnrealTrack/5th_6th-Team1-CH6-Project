// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Building/BaseBuilding.h"
#include "BaseCore.generated.h"

/**
 * 
 */
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

	void FindAnchors();

	UPROPERTY(EditDefaultsOnly)
	int32 ScanCount = 72;

	UPROPERTY(VisibleAnywhere)
	TArray<FVector> Anchors;
};
