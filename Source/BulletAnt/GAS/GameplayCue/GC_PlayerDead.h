#pragma once

#include "CoreMinimal.h"
#include "GameplayCueNotify_Actor.h"
#include "GC_PlayerDead.generated.h"

UCLASS()
class BULLETANT_API AGC_PlayerDead : public AGameplayCueNotify_Actor
{
	GENERATED_BODY()
	
public:
	virtual void HandleGameplayCue(AActor* MyTarget, EGameplayCueEvent::Type EventType, const FGameplayCueParameters& Parameters) override;
	
};
