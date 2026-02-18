#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "Mining/VoxelData.h"
#include "BAGameState.generated.h"

class ABaseCore;
class ABAPlayerController;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnOreChanged, EVoxelType, OreType, int32, OreCount);

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
	void SetOreCount(EVoxelType OreType, int32 Count);
	int32 GetOreCount(EVoxelType OreType);

	void BindOnOreChanged(const FOnOreChanged::FDelegate& Delegate);
	void UnbindOnOreChanged(const UObject* Object);

private:
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_UpdateOreCount(EVoxelType OreType, int32 Count);

private:
	UPROPERTY()
	TMap<EVoxelType, int32> OreInventory;

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
};
