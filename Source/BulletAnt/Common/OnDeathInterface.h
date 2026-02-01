#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "OnDeathInterface.generated.h"

UINTERFACE(MinimalAPI, BlueprintType)
class UOnDeathInterface : public UInterface
{
	GENERATED_BODY()
};

class BULLETANT_API IOnDeathInterface
{
	GENERATED_BODY()

public:
	UFUNCTION()
	virtual void OnDeath() = 0;
};
