
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "BulletPoolSubsystem.generated.h"

class ABaseProjectile;

USTRUCT()
struct FProjectilePool
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TSubclassOf<ABaseProjectile> ProjectileClass;

	UPROPERTY()
	TArray<ABaseProjectile*> Pool;
};

UCLASS()
class BULLETANT_API UBulletPoolSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	ABaseProjectile* GetProjectile(TSubclassOf<ABaseProjectile> ProjectileClass, UObject* Owner);

protected:
	UPROPERTY()
	TArray<FProjectilePool> Pools;

	ABaseProjectile* SpawnNewProjectile(TSubclassOf<ABaseProjectile> ProjectileClass, UObject* Owner);
};
