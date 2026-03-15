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
	UStaticMeshComponent* Mesh;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UProjectileMovementComponent* ProjectileMovement;

public:	
	ABAItemBox();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	FORCEINLINE bool GetbIsUsed() { return bIsUsed; };
	FORCEINLINE void SetbIsUsed(bool InIsUsed) { bIsUsed = InIsUsed; };

	virtual void Use_Implementation(AActor* User) override;

	UFUNCTION(BlueprintCallable)
	void SetItem(TSubclassOf<ABaseWeapon> InItem);

	UFUNCTION(BlueprintCallable)
	void DestroyItemBox();

protected:
	virtual void BeginPlay() override;

	UPROPERTY()
	TSubclassOf<ABaseWeapon> Item;

	UPROPERTY(Replicated)
	bool bIsUsed = false;

};
