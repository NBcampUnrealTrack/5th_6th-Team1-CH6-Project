#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "BAPlayerState.generated.h"

UCLASS()
class BULLETANT_API ABAPlayerState : public APlayerState
{
	GENERATED_BODY()

protected:
	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const override;

public:

public:
};
