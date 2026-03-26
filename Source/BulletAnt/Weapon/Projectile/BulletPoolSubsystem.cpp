#include "Weapon/Projectile/BulletPoolSubsystem.h"

#include "Weapon/Projectile/BaseProjectile.h"

ABaseProjectile* UBulletPoolSubsystem::GetProjectile(TSubclassOf<ABaseProjectile> ProjectileClass, UObject* Owner)
{
    if (!ProjectileClass) return nullptr;

    FProjectilePool* FoundPool = Pools.FindByPredicate([ProjectileClass](const FProjectilePool& Pool) {
        return Pool.ProjectileClass == ProjectileClass;
        });

    if (!FoundPool)
    {
        FProjectilePool NewPool;
        NewPool.ProjectileClass = ProjectileClass;
        Pools.Add(NewPool);
        FoundPool = &Pools.Last();
    }

    for (ABaseProjectile* Prj : FoundPool->Pool)
    {
        if (Prj && !Prj->GetbIsActive()) return Prj;
        
    }

    ABaseProjectile* NewPrj = SpawnNewProjectile(ProjectileClass, Owner);
    if (NewPrj)
    {
        FoundPool->Pool.Add(NewPrj);
    }

    return NewPrj;
}

ABaseProjectile* UBulletPoolSubsystem::SpawnNewProjectile(TSubclassOf<ABaseProjectile> ProjectileClass, UObject* Owner)
{
    if (!ProjectileClass || !GetWorld()) return nullptr;

    FActorSpawnParameters Params;
    Params.Owner = Cast<AActor>(Owner);
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    ABaseProjectile* NewPrj = GetWorld()->SpawnActor<ABaseProjectile>(ProjectileClass, FVector::ZeroVector, FRotator::ZeroRotator, Params);

    if (NewPrj)
    {
        NewPrj->SetActorHiddenInGame(true);
        NewPrj->SetActorEnableCollision(false);
    }

    return NewPrj;
}
