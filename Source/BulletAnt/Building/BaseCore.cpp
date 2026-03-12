// Fill out your copyright notice in the Description page of Project Settings.


#include "Building/BaseCore.h"
#include "Framework/BAGameState.h"

const TArray<FVector>& ABaseCore::GetAnchors() const
{
	return Anchors;
}

ABaseCore::ABaseCore()
{
    StaticMeshComp->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
    StaticMeshComp->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel6, ECollisionResponse::ECR_Block); // Enemy
}

void ABaseCore::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		UWorld* World = GetWorld();
		if (IsValid(World))
		{
			if (ABAGameState* GS = World->GetGameState<ABAGameState>())
			{
				GS->SetTargetCore(this);
			}
		}

		FindAnchors();
	}
}

void ABaseCore::FindAnchors()
{
    Anchors.Reserve(ScanCount);

    const float ScanRadius = 1000.f; 
    FVector Start = GetActorLocation();
    Start.Z = 0;

    FCollisionObjectQueryParams ObjectParams;
    ObjectParams.AddObjectTypesToQuery(ECC_WorldDynamic);
    for (int32 i = 0; i < ScanCount; ++i)
    {
        float Angle = i * (360.f / ScanCount);
        FVector Direction = FRotator(0.f, Angle, 0.f).Vector();
        FVector End = Start + Direction * ScanRadius;
        FHitResult Hit;
        if (GetWorld()->LineTraceSingleByObjectType(Hit, End, Start, ObjectParams))
        {
            Anchors.Add(Hit.Location);
        }
    }
}
