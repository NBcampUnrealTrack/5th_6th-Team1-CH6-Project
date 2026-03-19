#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "BAGameMode.generated.h"

enum class EOreType;

UCLASS()
class BULLETANT_API ABAGameMode : public AGameMode
{
	GENERATED_BODY()
	
public:
	ABAGameMode();

	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;

	virtual void HandleSeamlessTravelPlayer(AController*& C) override;

public:
	void MineOre(EOreType OreType, int32 PointCount);
	bool TrySpendOre(const TMap<EOreType, int32>& Cost);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 OreMultiplierMin = 15;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 OreMultiplierMax = 20;
};
