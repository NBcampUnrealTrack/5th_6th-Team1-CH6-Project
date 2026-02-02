#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "PMWGameModeBase.generated.h"

UCLASS()
class BULLETANT_API APMWGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
	
protected:
	virtual void BeginPlay() override;
};
