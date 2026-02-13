// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BAParkourComponent.generated.h"

class UMotionWarpingComponent;
UENUM(BlueprintType)
enum class EParkourType : uint8
{
	None,
	Climb,
	Vault
};
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BULLETANT_API UBAParkourComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UBAParkourComponent();
	bool AttemptParkour();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
protected:
	UFUNCTION(Server, Reliable)
	void ServerRPC_AttemptParkour();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_ExecuteParkour(EParkourType ParkourType, FVector TargetLocation, FRotator TargetRotation);
private:
	void DetectWall();
	void ExecuteParkour(EParkourType ParkourType);
	void OnParkourMontageEnded(UAnimMontage* Montage, bool bInterrupted);

public:
	UPROPERTY(EditAnywhere, Category = "Parkour|Montages")
	UAnimMontage* ClimbMontage;

	UPROPERTY(EditAnywhere, Category = "Parkour|Montages")
	UAnimMontage* VaultMontage;
private:

	UPROPERTY()
	UMotionWarpingComponent* MotionWarpingComp;
	UPROPERTY(EditAnywhere, Category = "Parkour")
	float TraceDistance = 100.f;
	UPROPERTY(EditAnywhere, Category = "Parkour")
	float HighTraceHeight = 200.f;
	UPROPERTY(EditAnywhere, Category = "Parkour")
	bool bDrawDebug = true;

	EParkourType CurrentParkourType;

	FHitResult WallHitResult;

	float WallHeight;
	float WallThickness;
	FVector WarpTargetLocation;
	FRotator WarpTargetRotation;
};
