// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ParkourComponent.generated.h"

class ACharacter;
class UCharacterMovementComponent;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class BULLETANT_API UParkourComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UParkourComponent();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// 클라이밍 시작
	UFUNCTION(BlueprintCallable, Category = "Parkour")
	void StartClimb(FVector TargetLocation);

	// 클라이밍 종료
	UFUNCTION(BlueprintCallable, Category = "Parkour")
	void EndClimb();

	// 클라이밍 상태 확인
	UFUNCTION(BlueprintPure, Category = "Parkour")
	bool IsClimbing() const { return bIsClimbing; }

protected:
	UPROPERTY()
	TObjectPtr<ACharacter> OwnerCharacter;

	UPROPERTY()
	TObjectPtr<UCharacterMovementComponent> MovementComponent;

	// 클라이밍 상태
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Parkour")
	bool bIsClimbing = false;

	// 클라이밍 시작/종료 위치
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Parkour")
	FVector ClimbStartLocation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Parkour")
	FVector ClimbEndLocation;

	// 클라이밍 지속 시간 (에디터에서 조정 가능)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parkour")
	float ClimbDuration = 1.f;

	// 클라이밍 타이머
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Parkour")
	float ClimbTimer = 0.f;

public:
	// 파쿠르 감지 설정
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Parkour")
	float TraceDistance = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Parkour")
	float EyeHeight = 50.f;
};
