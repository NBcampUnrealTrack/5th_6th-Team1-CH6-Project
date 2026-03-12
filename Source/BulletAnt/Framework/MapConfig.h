#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "MapConfig.generated.h"

UCLASS()
class BULLETANT_API UMapConfig : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadonly)
	TSoftObjectPtr<UWorld> LobbyLevel;

	UPROPERTY(EditAnywhere, BlueprintReadonly)
	TArray<TSoftObjectPtr<UWorld>> GameLevels;
};
