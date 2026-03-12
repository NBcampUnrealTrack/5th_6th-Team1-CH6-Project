// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Common/BAItemInterface.h"
#include "BAItemBox.generated.h"

class UProjectileMovementComponent;
class ABaseWeapon;

UCLASS()
class BULLETANT_API ABAItemBox : public AActor, public IBAItemInterface
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	USceneComponent* Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* Mesh;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UProjectileMovementComponent* ProjectileMovement;

public:	
	ABAItemBox();

	virtual void Use_Implementation(AActor* User) override;

	UFUNCTION(BlueprintCallable)
	void SetItem(TSubclassOf<ABaseWeapon> InItem);

protected:
	virtual void BeginPlay() override;

	UPROPERTY()
	TSubclassOf<ABaseWeapon> Item;



};
