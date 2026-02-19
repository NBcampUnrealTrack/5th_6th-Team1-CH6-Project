// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BAPlayerController.generated.h"


class UInputMappingContext;
class ABuildingManagerComponent;
class UUW_PlayerHUDWidget;

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

	FORCEINLINE UUW_PlayerHUDWidget* GetHUD() { return HUD; };
protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UUW_PlayerHUDWidget> HUDClass;

	UPROPERTY()
	TObjectPtr<UUW_PlayerHUDWidget> HUD;
	
#pragma region GroundScanner

public:
	void SwitchGroundScanner();

protected:
	uint8 bActiveGroundScannerUI : 1 = false;

#pragma endregion

};
