#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "BAGameMode.generated.h"

enum class EVoxelType : uint8;

UCLASS()
class BULLETANT_API ABAGameMode : public AGameMode
{
	GENERATED_BODY()
	
public:
	ABAGameMode();

	virtual void PostLogin(APlayerController* NewPlayer) override;

	virtual void Logout(AController* Exiting) override;

	void MineOre(EVoxelType OreType, int32 PointCount);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 OreMultiplierMin = 15;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 OreMultiplierMax = 20;
};
