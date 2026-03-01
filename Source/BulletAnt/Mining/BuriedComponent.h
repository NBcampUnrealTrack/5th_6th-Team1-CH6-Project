#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Mining/VoxelData.h"
#include "BuriedComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BULLETANT_API UBuriedComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	void GetPredictedBoundInfos(TArray<FBuryBoundInfo>& OutInfos, const FTransform& SpawnTransform) const;
	FORCEINLINE bool IsCarveDensity() const { return bCarveDensity; }

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	uint8 bCarveDensity : 1 = true;
};
