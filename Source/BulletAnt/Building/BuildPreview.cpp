// Fill out your copyright notice in the Description page of Project Settings.


#include "Building/BuildPreview.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

ABuildPreview::ABuildPreview()
{
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	SetRootComponent(MeshComp);

	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ABuildPreview::InitWithData(const FBuildingRow& Row)
{
    MeshComp->SetStaticMesh(Row.PreviewMesh);
    PlacementBoxExtent = Row.PlacementBoxExtent;

    if (PreviewBaseMaterial)
    {
        MID = UMaterialInstanceDynamic::Create(PreviewBaseMaterial, this);
        const int32 NumMaterials = MeshComp->GetNumMaterials();
        for (int32 i = 0; i < NumMaterials; ++i)
        {
            MeshComp->SetMaterial(i, MID);
        }
    }

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
