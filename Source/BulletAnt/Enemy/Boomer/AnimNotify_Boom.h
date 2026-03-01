// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "GameplayTagContainer.h"
#include "AnimNotify_Boom.generated.h"

/**
 * 
 */
UCLASS()
class BULLETANT_API UAnimNotify_Boom : public UAnimNotify
{
	GENERATED_BODY()
	
public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GAS")
	FGameplayTag GameplayCueBoomTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GAS")
	FName SocketName;
};
