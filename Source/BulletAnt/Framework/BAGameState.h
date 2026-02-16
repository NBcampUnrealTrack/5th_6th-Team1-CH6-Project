#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "BAGameState.generated.h"

enum class EVoxelType : uint8;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnOreChanged, EVoxelType, OreType, int32, OreCount);

UCLASS()
class BULLETANT_API ABAGameState : public AGameState
{
	GENERATED_BODY()
	
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
};
