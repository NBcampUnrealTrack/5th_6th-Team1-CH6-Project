// Fill out your copyright notice in the Description page of Project Settings.


#include "Building/BuildPreview.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

ABuildPreview::ABuildPreview()
{
    bReplicates = false;

    StaticMeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    BuildingBounds->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    BuildingBounds->SetGenerateOverlapEvents(false);
}

void ABuildPreview::InitWithData(const FBuildingRow& Row)
{
    StaticMeshComp->SetStaticMesh(Row.PreviewMesh);
    if (PreviewBaseMaterial)
    {
        MID = UMaterialInstanceDynamic::Create(PreviewBaseMaterial, this);
        const int32 NumMaterials = StaticMeshComp->GetNumMaterials();
        for (int32 i = 0; i < NumMaterials; ++i)
        {
            StaticMeshComp->SetMaterial(i, MID);
        }
    }

    ApplyBuildingBounds(Row.BuildingBoxExtent);

    SetCanPlace(false);
}

void ABuildPreview::UpdateTransform(const FVector& Location, const FRotator& Rotation)
{
    SetActorLocationAndRotation(Location, Rotation);
}

void ABuildPreview::SetCanPlace(bool bInCanPlace)
{
    bCanPlace = bInCanPlace;

    if (!MID) 
    {
        return;
    }

    const float Opacity = 0.7f;
    const FLinearColor CanColor(0.f, 1.f, 0.f, Opacity);
    const FLinearColor BlockColor(1.f, 0.f, 0.f, Opacity);

    MID->SetVectorParameterValue(TEXT("ActorColor"), bCanPlace ? CanColor : BlockColor);
}
