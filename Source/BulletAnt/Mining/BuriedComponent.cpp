#include "Mining/BuriedComponent.h"
#include "Components/BoxComponent.h"

void UBuriedComponent::GetPredictedBoundInfos(TArray<FBuryBoundInfo>& OutInfos, const FTransform& SpawnTransform) const
{
	TArray<UBoxComponent*> Boxes;
	GetOwner()->GetComponents<UBoxComponent>(Boxes);

	for (const auto& Box : Boxes)
	{
		if (IsValid(Box) == false)
			continue;

		FBuryBoundInfo Info;
		Info.Transform = SpawnTransform * Box->GetRelativeTransform();
		Info.Extent = Box->GetScaledBoxExtent();
		FBox LocalBox(-Info.Extent, Info.Extent);
		Info.Bound = LocalBox.TransformBy(Info.Transform);
		OutInfos.Add(Info);
	}
}

