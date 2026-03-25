#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "BAGameInstance.generated.h"

class UMapConfig;

UCLASS()
class BULLETANT_API UBAGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	virtual void OnStart() override;

	UMapConfig* GetMapConfig() { return MapConfig; }

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UMapConfig> MapConfig;
};
