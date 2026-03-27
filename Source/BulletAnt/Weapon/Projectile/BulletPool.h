#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BulletPool.generated.h"

class BaseProjectile;

UCLASS()
class BULLETANT_API ABulletPool : public AActor
{
	GENERATED_BODY()
	
public:	
	ABulletPool();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

};
