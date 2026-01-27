#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "FireStartInterface.generated.h"

UINTERFACE(BlueprintType)
class BULLETANT_API UFireStartInterface : public UInterface
{
	GENERATED_BODY()
};

class BULLETANT_API IFireStartInterface
{
	GENERATED_BODY()

public:
	virtual FVector GetFireStartLocation() const = 0;

	virtual FVector GetFireDirection() const = 0;
};
