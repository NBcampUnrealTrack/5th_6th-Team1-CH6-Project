// Fill out your copyright notice in the Description page of Project Settings.


#include "Building/BaseBox.h"

void ABaseBox::GetEdgesLocal(TArray<FBuildingEdge>& OutEdges) const
{
	Super::GetEdgesLocal(OutEdges);

	const float X = BuildingBoxExtent.X;
	const float Y = BuildingBoxExtent.Y;
	const float TopZ = BuildingBoxExtent.Z * 2.0f;

	const FVector P0(+X, +Y, TopZ);
	const FVector P1(+X, -Y, TopZ);
	const FVector P2(-X, -Y, TopZ);
	const FVector P3(-X, +Y, TopZ);

	OutEdges.Add({ P0, P1 });
	OutEdges.Add({ P1, P2 });
	OutEdges.Add({ P2, P3 });
	OutEdges.Add({ P3, P0 });
}
