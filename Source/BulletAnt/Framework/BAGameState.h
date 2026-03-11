#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "Mining/VoxelData.h"
#include "BAGameState.generated.h"

class ABaseCore;
class ABAPlayerController;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnOreChanged, EOreType, OreType, int32, OreCount);

DECLARE_MULTICAST_DELEGATE(FOnWaveTimeChanged);

UCLASS()
class BULLETANT_API ABAGameState : public AGameState
{
	GENERATED_BODY()

public:
	ABAGameState();

	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

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

protected:
	UPROPERTY()
	TObjectPtr<ABaseCore> TargetCore;

	UPROPERTY()
	TArray<TObjectPtr<ABAPlayerController>> ConnectedPlayers;

#pragma endregion

#pragma region Wave
public:
	int32 GetInitWavePreparationTime() const;
	void SetInitWavePreparationTime(int32 InTime);

	int32 GetWavePreparationTime() const;
	void SetWavePreparationTime(int32 InTime);

	UFUNCTION()
	void OnRep_WavePreparationTime();

protected:
	UPROPERTY(Replicated)
	int32 InitWavePreparationTime;

	UPROPERTY(Replicated, ReplicatedUsing = OnRep_WavePreparationTime)
	int32 WavePreparationTime;

public:
	FOnWaveTimeChanged OnWaveTimeChanged;

#pragma endregion
};
