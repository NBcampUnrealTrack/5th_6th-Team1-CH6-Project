#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "DataAssetInterface.generated.h"

UINTERFACE(MinimalAPI, BlueprintType)
class UDataAssetInterface : public UInterface
{
	GENERATED_BODY()
};

class BULLETANT_API IDataAssetInterface
{
	GENERATED_BODY()

public:
	virtual UDataAsset* GetDataAsset() const = 0;
};
