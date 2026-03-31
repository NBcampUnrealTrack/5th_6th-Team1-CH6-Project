// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BAPlayerController.generated.h"

UENUM()
enum class ELevelType : uint8
{
	Lobby,
	Main,
};

class UInputMappingContext;
class ABuildingManagerComponent;
class UUW_PlayerHUDWidget;
class UUW_RespawnBar;
class ABACharacter;
class UUISubsystem;
class UUW_WaveTimer;
class ABaseShop;
class ABaseWeapon;
class UUW_ShopWindow;
class ABAItemBox;
class UWeaponDataAsset;

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
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;


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
#pragma region Weapon
public:

	void ShowAmmo();
	void HideAmmo();
protected:

#pragma endregion

#pragma region Shop

public:
	UFUNCTION(Server, Reliable)
	void Server_RequestBuyGacha(ABaseShop* InShop, int32 GachaID, int32 Count);

	UFUNCTION(Server, Reliable)
	void Server_RequestAddWeapon(ABAItemBox* InItemBox);

	UFUNCTION()
	void RequestDeleteBox(ABAItemBox* InItemBox);

	UFUNCTION()
	void ShowShopUI();

#pragma endregion
	
#pragma region GroundScanner

public:
	void SwitchGroundScanner();

protected:
	uint8 bActiveGroundScannerUI : 1 = false;

#pragma endregion

#pragma region SeamlessTravel

public:
	void SetLevelType(ELevelType InType);

	UFUNCTION(Client, Reliable)
	void Client_RemoveRefreshedVoice(const FString& IdToRemove);

protected:
	UFUNCTION(Client, Reliable)
	void Client_SetupController(ELevelType InType);
	void SetupController();

	void SetupForLobby();
	void SetupForMain();

protected:
	ELevelType LevelType = ELevelType::Lobby;

	TArray<FString> ToRemoveRefresedVoices;

#pragma endregion

#pragma region IntializeMain

public:
	UFUNCTION(Server, Reliable)
	void Server_ReadyToStart();
	UFUNCTION(Client, Reliable)
	void Client_StartGame();

#pragma endregion

#pragma region WaveTimer

protected:
	UPROPERTY()
	TObjectPtr<UUW_WaveTimer> WaveTimerUI;

#pragma endregion
};
