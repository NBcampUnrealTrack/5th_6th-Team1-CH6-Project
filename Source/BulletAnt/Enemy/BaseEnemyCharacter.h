// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"	
#include "GameFramework/Character.h"
#include "BaseEnemyCharacter.generated.h"

class UStateTreeComponent;

UCLASS()
class BULLETANT_API ABaseEnemyCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ABaseEnemyCharacter();
	
	AActor* GetTargetActor() const;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<AActor> TargetActor;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	float AcceptanceRadius;
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UStateTreeComponent> StateTreeComponent;
};
