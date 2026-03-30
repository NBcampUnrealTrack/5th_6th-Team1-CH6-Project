#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "Mining/VoxelData.h"
#include "BAGameState.generated.h"

class ABaseCore;
class ABAPlayerController;
class ABaseWeapon;
class ABACharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnOreChanged, EOreType, OreType, int32, OreCount);
DECLARE_MULTICAST_DELEGATE(FOnWaveTimeChanged);
DECLARE_MULTICAST_DELEGATE(FOnRemainingEnemy);

UCLASS()
class BULLETANT_API ABAGameState : public AGameState
{
	GENERATED_BODY()

public:
	ABAGameState();

	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

public:
	void AddActiveCharacter(ABACharacter* InCharacter);
	void RemoveActiveCharacter(ABACharacter* InCharacter);

	const TArray<TWeakObjectPtr<ABACharacter>>& GetActiveCharacters() const { return ActiveCharacters; }

protected:
	UPROPERTY()
	TArray<TWeakObjectPtr<ABACharacter>> ActiveCharacters;

#pragma region Ground

public:
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_EditGround(const struct FVoxelChunkEditPacket& Packet);
	UFUNCTION()
	void OnRep_SetInitParams();

private:
	UPROPERTY(ReplicatedUsing = OnRep_SetInitParams)
	FGroundInitializeParams GroundInitParams;

#pragma endregion

#pragma region Ore

public:
	void SetOreCount(EOreType OreType, int32 Count);
	int32 GetOreCount(EOreType OreType);
	bool CanPurchase(const TMap<EOreType, int32>& Cost);
	FORCEINLINE const TMap<EOreType, int32>& GetOreInventory() const { return OreInventory; }

	void BindOnOreChanged(const FOnOreChanged::FDelegate& Delegate);
	void UnbindOnOreChanged(const UObject* Object);

private:
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_UpdateOreCount(EOreType OreType, int32 Count);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TMap<EOreType, int32> OreInventory;

	UPROPERTY()
	FOnOreChanged OnOreChanged;

#pragma endregion

#pragma region Enemy
public:
	ABaseCore* GetTargetCore() const;

	void SetTargetCore(ABaseCore* InTargetCore);

	void AddPlayerController(ABAPlayerController* NewPlayer);

	void RemovePlayerController(ABAPlayerController* ExitingPlayer);

	TArray<ABAPlayerController*> GetAllPlayerControllers() const;

	ABAPlayerController* GetPlayerControllerByIndex(int32 Index) const;

	int32 GetRemainingEnemy() const;

	void SetRemainingEnemy(int32 InRemainingEnemy);

	UFUNCTION()
	void OnRep_RemainingEnemy();

protected:
	UPROPERTY()
	TObjectPtr<ABaseCore> TargetCore;

	UPROPERTY()
	TArray<TObjectPtr<ABAPlayerController>> ConnectedPlayers;

	UPROPERTY(Replicated, ReplicatedUsing = OnRep_RemainingEnemy)
	int32 RemainingEnemy = 0;

public:
	FOnRemainingEnemy OnRemainingEnemy;

#pragma endregion

#pragma region Wave
public:
	int32 GetInitWavePreparationTime() const;
	void SetInitWavePreparationTime(int32 InTime);

	int32 GetWavePreparationTime() const;
	void SetWavePreparationTime(int32 InTime);

	int32 GetSpawnTime() const;
	void SetSpawnTime(int32 InTime);

	int32 GetDate() const;
	void SetDate(int32 InDate);

	int32 GetFinalDate() const;
	void SetFinalDate(int32 InDate);

	UFUNCTION()
	void OnRep_WavePreparationTime();

	UFUNCTION()
	void OnRep_Date();

protected:
	UPROPERTY(Replicated)
	int32 InitWavePreparationTime;

	UPROPERTY(Replicated, ReplicatedUsing = OnRep_WavePreparationTime)
	int32 WavePreparationTime;

	UPROPERTY(Replicated)
	int32 SpawnTime;

	UPROPERTY(Replicated, ReplicatedUsing = OnRep_Date)
	int32 Date;

	UPROPERTY(Replicated)
	int32 FinalDate;

public:
	FOnWaveTimeChanged OnWaveTimeChanged;

#pragma endregion

#pragma region Weapon

public:
	UFUNCTION()
	void AddHaveWeapon(TSubclassOf<ABaseWeapon> InWeaponClass);

	FORCEINLINE const TArray<TSubclassOf<ABaseWeapon>>& GetHaveWeaponArray() { return HaveWeaponArray; };

protected:
	UPROPERTY(Replicated)
	TArray<TSubclassOf<ABaseWeapon>> HaveWeaponArray;

	UPROPERTY(EditDefaultsOnly)
	TArray<TSubclassOf<ABaseWeapon>> InitWeaponArray;

#pragma endregion
};
