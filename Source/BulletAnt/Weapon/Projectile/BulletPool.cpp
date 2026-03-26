#include "Weapon/Projectile/BulletPool.h"

ABulletPool::ABulletPool()
{
	PrimaryActorTick.bCanEverTick = true;

}

void ABulletPool::BeginPlay()
{
	Super::BeginPlay();	

}

// Called every frame
void ABulletPool::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

