#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "BAGameInstance.generated.h"

UCLASS()
class BULLETANT_API UBAGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	virtual void Init() override;

private:
	void EpicLogin();
};
