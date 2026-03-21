#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PlayerColorSubsystem.generated.h"

UCLASS()
class BULLETANT_API UPlayerColorSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	int32 GetColorIndex(FUniqueNetIdRepl UserId);
	void ReleaseColorIndex(FUniqueNetIdRepl UserId);

protected:
	UPROPERTY()
	TMap<FUniqueNetIdRepl, int32> NetIdColorIdxMap;

	UPROPERTY()
	uint8 UsingColorBitmap = 0;
};
