#include "GAS/GameplayCue/GC_PlayerDead.h"

#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"

void AGC_PlayerDead::HandleGameplayCue(AActor* MyTarget, EGameplayCueEvent::Type EventType, const FGameplayCueParameters& Parameters)
{
    if (!MyTarget) return;

    ACharacter* Character = Cast<ACharacter>(MyTarget);
    if (!Character) return;

    USkeletalMeshComponent* Mesh = Character->GetMesh();
    UCapsuleComponent* Capsule = Character->GetCapsuleComponent();

    switch (EventType)
    {
    case EGameplayCueEvent::OnActive:
        Mesh->SetSimulatePhysics(true);
        Mesh->SetCollisionProfileName(TEXT("Ragdoll"));
        return;

    case EGameplayCueEvent::Removed:
        Mesh->SetSimulatePhysics(false);
        Mesh->SetCollisionProfileName(TEXT("Pawn"));
        return;

    default:
        return;
    }
}
