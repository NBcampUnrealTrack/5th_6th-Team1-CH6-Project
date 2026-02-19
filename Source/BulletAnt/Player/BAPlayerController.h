// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BAPlayerController.generated.h"


class UInputMappingContext;
class ABuildingManagerComponent;
class UUW_PlayerHUDWidget;
class UUW_RespawnBar;
class ABACharacter;
class UUISubsystem;

UCLASS()
class BULLETANT_API ABAPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	// --- [블루프린트 입력용 변수]
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputMappingContext* DefaultMappingContext;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Building")
	UInputMappingContext* BuildingMappingContext;

	bool bIsBuildMode;

	void SwitchingMode();

	UFUNCTION()
	void StartRespawnBar(float InTotalTime);

	UFUNCTION()
	void StopRespawnBar();

	FORCEINLINE UUW_PlayerHUDWidget* GetHUD() { return HUD; };
protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void HandleRespawnBar();



	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UUW_PlayerHUDWidget> HUDClass;

	UPROPERTY()
	TObjectPtr<UUISubsystem> UISubsystem;

	UPROPERTY()
	TObjectPtr<UUW_PlayerHUDWidget> HUD;

	UPROPERTY()
	TObjectPtr<UUW_RespawnBar> RespawnBarUI;

	UPROPERTY()
	float TotalTime;

	UPROPERTY()
	float CurrentTime;

	FTimerHandle RespawnBarTimer;
};
